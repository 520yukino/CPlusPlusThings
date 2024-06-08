#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/* 相比于基本模型，这里使用多进程复制了多个客户端来同时连接，并在关闭客户端前等待了一些时间 */

#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    int clisock;
    struct sockaddr_in seraddr;
    char message[] = "I'm client, Hello World!";
    const int SIZE = 1024;
    char *buf = (char *)malloc(SIZE);
    pid_t pid;

    if (argc != 3) //需要传入IP和端口号
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    
    for (int i = 0; i < 3; i++) //2^循环次数=总进程数
        pid = fork(); //需在创建套接字之前fork，因为套接字不能复制。由于客户端是在connect中自动绑定的，所以不会有端口冲突

    if ((clisock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr(args[1]);
    seraddr.sin_port = htons(atoi(args[2]));

    if (connect(clisock, (struct sockaddr *)(&seraddr), sizeof(seraddr)) == -1) //多个进程同时连接一个服务端，
        errorputs("connect() failed");
    puts("Client: send message");
    write(clisock, message, sizeof(message));

    sleep(2);
    if (read(clisock, buf, SIZE) == -1)
        errorputs("read() failed!");
    //此处需要注意，如果客户端使用close或中断来彻底关闭，并且之前没有使用read函数，那么服务端的读取操作会返回-1，但使用shutdown关闭或者调用了read则正常返回0，待探究
    printf("From server: %s\n", buf); //由于只读入了1次服务端发来的数据，所以此处输出为服务端发来的第1个字符串
    shutdown(clisock, SHUT_RDWR);
    close(clisock);
    usleep(10000); //等待所有子进程输出完毕

    return 0;
}