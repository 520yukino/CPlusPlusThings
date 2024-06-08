//全局聊天的服务端，接收客户端信息后发送至所有客户端
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#pragma GCC diagnostic ignored "-Wformat-overflow" //消除sprintf中会出现的内存超出警告

#define ErrorPuts(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

#define BUF_SIZE 1024 //普通信息缓存区大小
#define BUF_SIZE_SMALL 128 //小号信息缓存区大小
#define SERVER_NAME_STR "WSL Ubuntu web server" //服务端名称

//每个客户端连接后创键的线程
void *FT_Client(void *);
//获取需要传输的数据类型
char *F_GetContentType(char *filename);
//传输文件响应信息
void F_SendData(int sock, char *conttype, char *filename);
//传输错误响应信息
void F_SendError(int sock);
//关闭套接字等相关操作
void F_CloseSock(int sock);

pthread_mutex_t mutex; //锁住全局变量
int clicount = 0; //客户端数量

int main(int argc, char *args[])
{
    int sersock, clisock;
    struct sockaddr_in seraddr, cliaddr;
    socklen_t szcliaddr = sizeof(cliaddr);
    pthread_t tid;
    int sockopt = 1, optlen = sizeof(sockopt); //设置套接字选项所需变量

    if (argc != 2) {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    
    pthread_mutex_init(&mutex, NULL);
    if ((sersock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        ErrorPuts("socket() failed!");
    if (setsockopt(sersock, SOL_SOCKET, SO_REUSEADDR, (char *)&sockopt, optlen)) //设置可复用处于TIME_WAIT状态端口的服务端
        ErrorPuts("setsockopt() 1 error!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (struct sockaddr *)(&seraddr), sizeof(seraddr)) == -1)
        ErrorPuts("bind() failed!");
    if (listen(sersock, 3) == -1)
        ErrorPuts("listen() failed!");

    while (1) //循环创建客户端连接以及客户端线程
    {
        if ((clisock = accept(sersock, (struct sockaddr *)(&cliaddr), &szcliaddr)) == -1)
            ErrorPuts("accept() failed!");
        pthread_mutex_lock(&mutex);
        clicount++;
        pthread_mutex_unlock(&mutex);
        printf("Server: client %d connect, ip/port: %s:%d - exist %d\n", clisock, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), clicount);
        pthread_create(&tid, NULL, FT_Client, (void *)clisock); //直接传值而非地址，这可以让实参改变时形参不受影响
        pthread_detach(tid);
    }

    close(sersock);
    return 0;
}

void *FT_Client(void *arg)
{
    int sock = (int)arg; //客户端的套接字
    int res;
    char reqline[BUF_SIZE], filename[BUF_SIZE_SMALL], conttype[BUF_SIZE_SMALL];

    //接收信息并分离出请求行
    res = read(sock, reqline, BUF_SIZE);
    strtok(reqline, "\r\n");
    if (!res) {
        F_CloseSock(sock);
        return NULL;
    }
    else if (res == -1)
        ErrorPuts("read() failed!");
    printf("clinet %d request line: %s\n", sock, reqline);
    
    //下面检查请求行格式，若错误则发送错误信息并返回。分别检查协议版本信息和请求方式，strstr返回参数1中参数2所在位置，没有就返回NULL；strtok返回参数1由参数2中2个分隔符分割的第一部分
    if (!strstr(reqline, "HTTP/") || strcmp(strtok(reqline, " "), "GET")) {
        F_SendError(sock); //传输错误信息
        fprintf(stderr, "Warning: client %d error request line\n", sock);
        return NULL;
    }
    strcpy(filename, strtok(NULL, " ")+1); //分离出请求的文件名，传入NULL表示继续分解之前的字符串，这里前移了一个字符来移除'/'
    strcpy(conttype, F_GetContentType(filename)); //根据文件名获取需要传输的类型
    F_SendData(sock, conttype, filename); //传输信息

    F_CloseSock(sock); //统一在这里关闭套接字
    return NULL;
}

char *F_GetContentType(char *filename)
{
    filename = strrchr(filename, '.')+1; //从后往前分离出后缀，前移一字符来移除'.'
    //根据后缀返回传输类型
    if (!strcmp(filename, "html") || !strcmp(filename, "htm"))
        return "text/html";
    else if (!strcmp(filename, "txt"))
        return "text/txt";
    else if (!strcmp(filename, "ico"))
        return "image/x-icon";
    else
        return "text/none";
}

void F_SendData(int sock, char *conttype, char *filename)
{
    printf("%s %s\n", conttype, filename);
    static char statline[] = "HTTP/1.1 200 OK\r\n"; //状态行
    char infohead[][BUF_SIZE_SMALL] = { //信息头
        "Date:",
        "Server:",
        "Content-Length:",
        "Content-Type:",
    };
    char buf[BUF_SIZE]; //充当临时字符串复制区以及文件读入缓存区
    FILE *fp = fopen(filename, "r");
    if (!fp) { //文件打开错误
        F_SendError(sock);
        return;
    }

    //构键日期字符串，通过ctime函数获取固定格式化的日期字符串，但需要清除末尾的换行符
    time_t timedate;
    char *tdstr;
    time(&timedate);
    tdstr = ctime(&timedate);
    *strchr(tdstr, '\n') = '\0';
    strcpy(buf, infohead[0]);
    sprintf(infohead[0], "%s %s UTC\r\n", buf, tdstr);
    //构建服务端名称
    strcpy(buf, infohead[1]);
    sprintf(infohead[1], "%s %s\r\n", buf, SERVER_NAME_STR);
    //构建传输字节数字符串，通过seek将文件指针当前位置移动到文件尾，再用tell获取当前位置相对文件头的偏移量。实际上不传输此数据也不会出错
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    strcpy(buf, infohead[2]);
    sprintf(infohead[2], "%s %ld\r\n", buf, filesize);
    //构建传输类型字符串，作为信息头的最后一行，需要额外添加一个空行以分割信息体
    strcpy(buf, infohead[3]);
    sprintf(infohead[3], "%s %s\r\n\r\n", buf, conttype);

    //传输响应信息，分别传输状态行、信息头、信息体(请求的文件)
    write(sock, statline, strlen(statline));
    for (int i = 0; i < sizeof(infohead)/sizeof(infohead[0]); i++)
        write(sock, infohead[i], strlen(infohead[i]));
    do { //利用fread而非fgets来一次性读入足够多的字节，而后使用feof判尾
        memset(buf, 0, sizeof(buf)); //重置为'\0'
        fread(buf, sizeof(buf)-1, 1, fp); //至少保留最后一个'\0'以作为strlen的根据
        write(sock, buf, strlen(buf));
    } while (!feof(fp));
}

void F_SendError(int sock)
{
    static char statline[] = "HTTP/1.1 400 Bad Request\r\n"; //状态行
    char infohead[][BUF_SIZE_SMALL] = { //信息头
        "Server:",
        "Content-Length:",
        "Content-Type: text/html\r\n\r\n"
    };
    static char infobody[] = "<html><head><title>NETWORK</title></head>"
        "<body><font size=+5><br> error! inspect your request file name and request method!"
        "</font></body></html>"; //信息体
    char buf[BUF_SIZE_SMALL];

    //构建服务端名称
    strcpy(buf, infohead[0]);
    sprintf(infohead[0], "%s %s\r\n", buf, SERVER_NAME_STR);
    //构建数据长度，此长度必须准确，如果小于信息体长度则无法传输完整信息，大于则直接无法被识别
    strcpy(buf, infohead[1]);
    sprintf(infohead[1], "%s %lu\r\n", buf, sizeof(infobody)-1);

    //传输错误响应信息
    write(sock, statline, strlen(statline));
    for (int i = 0; i < sizeof(infohead)/sizeof(infohead[0]); i++)
        write(sock, infohead[i], strlen(infohead[i]));
    write(sock, infobody, strlen(infobody));
}

void F_CloseSock(int sock)
{
    close(sock);
    pthread_mutex_lock(&mutex);
    clicount--;
    pthread_mutex_unlock(&mutex);
    printf("Server: client %d closed\n", sock);
}