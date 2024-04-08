#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
//recv函数的参数4为可选项，MSG_OOB为紧急带外传输模式，MSG_PEEK为观察并读取输入的内容但不清理，通常和MSG_DONTWAIT不阻塞模式一起使用
#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

const int SIZE = 1024;
int sersock, clisock;

void F_SignUrgent(int sig) //紧急信号处理函数
{
    int len;
    char buf[SIZE];
    len = recv(clisock, buf, sizeof(buf), MSG_OOB); //接收客户端的带外传输数据
    printf("Urgent message of %d byte: %s\n", len, buf);
}

int main(int argc, char* args[])
{
    struct sockaddr_in seraddr, cliaddr;
    socklen_t szcliaddr = sizeof(cliaddr);
    char reply_1[] = "connecting";
    char reply_2[] = "over";
    char* message = (char*)malloc(SIZE);
    int res;

    if (argc != 2)
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    struct sigaction sa; //设置紧急信息处理函数
    sa.sa_handler = F_SignUrgent;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if ((sersock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (struct sockaddr*)(&seraddr), sizeof(seraddr)) == -1)
        errorputs("bind() failed!");
    if (listen(sersock, 5) == -1)
        errorputs("listen() failed!");
    if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == -1)
        errorputs("accept() failed!");

    // int optval, optlen = sizeof(optval); //设置套接字选项，
    // getsockopt(sersock, SOL_SOCKET, SO_OOBINLINE, (char *)&optval, &optlen);
    // printf("SO_OOBINLINE = %d\n", optval);
    // optval = 1;
    // setsockopt(sersock, SOL_SOCKET, SO_OOBINLINE, (char *)&optval, optlen);
    fcntl(clisock, F_SETOWN, getpid()); //将clisock的事件信号接收者(包括SIGURG)设置为本进程(服务端)
    sigaction(SIGURG, &sa, NULL);

    puts("send reply_1");
    write(clisock, reply_1, sizeof(reply_1));
    while ((res = read(clisock, message, SIZE)) != 0)
    {
        if (res == -1)
            errorputs("read() failed!");
        printf("Message from client of %d byte: %s\n", res, message);
    }
    // while ((res = recv(clisock, message, SIZE, MSG_OOB)) != 0)
    // {
    //     if (res == -1) {
    //         perror("recv()");
    //         exit(-1);
    //     }
    //     printf("Message from client of %d byte: %s\n", res, message);
    // }

    puts("send reply_2");
    write(clisock, reply_2, sizeof(reply_2));
   
    close(clisock);
    close(sersock);
    return 0;
}