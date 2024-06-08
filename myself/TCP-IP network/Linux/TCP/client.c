#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/* 客户端基本模型 */
#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    int clisock; //套接字句柄
    struct sockaddr_in seraddr; //套接字地址信息
    char message[] = "I'm client, Hello World!";
    const int SIZE = 1024;
    char *buf = (char *)malloc(SIZE);

    if (argc != 3) //需要传入IP和端口号
    {
        printf("Usage: %s <IP> <Port>\n", args[0]);
        exit(-1);
    }
    
    if ((clisock = socket(PF_INET, SOCK_STREAM, 0)) == -1) //创建服务器端套接字
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr)); //设置seraddr
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr(args[1]);
    seraddr.sin_port = htons(atoi(args[2]));

    //最好不要自行绑定客户端地址信息，如果客户端主动关闭连接，则客户端的此端口号会保持TIME_WAIT状态，进而导致下一次的绑定失败
    // struct sockaddr_in cliaddr;
    // memset(&cliaddr, 0, sizeof(cliaddr));
    // cliaddr.sin_family = AF_INET;
    // cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // cliaddr.sin_port = htons(50000); //使用固定端口时
    // if (bind(clisock, (struct sockaddr*)(&cliaddr), sizeof(cliaddr)) == -1) //绑定套接字地址信息
    //     errorputs("bind() failed!");
    if (connect(clisock, (struct sockaddr *)(&seraddr), sizeof(seraddr)) == -1) //客户端发出连接请求，与win不同的是此处会立即返回而不做等待
        errorputs("connect() failed");
    if (read(clisock, buf, SIZE) == -1) //Linux也有send和recv函数，它们和win的函数几乎一致
        errorputs("read() failed!");
    printf("Reply from server: %s\n", buf);
    puts("send begin");
    write(clisock, message, sizeof(message));
    puts("send over");
    // if (write(clisock, message, sizeof(message)) == -1) //与win不同，服务端关闭后使用收发函数不会出错
    //     errorputs("write() failed!");

    close(clisock);
    // shutdown(clisock, SHUT_WR); //断开输出流保留输入流，后续依旧可以读入数据
    // if (read(clisock, buf, SIZE) == -1)
    //     errorputs("read() failed!");
    // printf("Reply from server: %s\n", buf);

    return 0;
}