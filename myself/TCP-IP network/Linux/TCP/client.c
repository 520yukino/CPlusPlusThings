#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    int clisock; //套接字句柄
    struct sockaddr_in seraddr; //套接字地址信息
    char message[8888] = "I'm client, Hello World!";
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

    // struct sockaddr_in cliaddr; //自行绑定客户端地址信息，如果客户端主动关闭连接，则客户端的此端口号会出入TIME_WAIT状态，一段时间内无法connect
    // memset(&cliaddr, 0, sizeof(cliaddr));
    // cliaddr.sin_family = AF_INET;
    // cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // cliaddr.sin_port = htons(50000); //使用固定端口时
    // if (bind(clisock, (struct sockaddr*)(&cliaddr), sizeof(cliaddr)) == -1) //绑定套接字地址信息
    //     errorputs("bind() failed!");

    if (connect(clisock, (struct sockaddr *)(&seraddr), sizeof(seraddr)) == -1) //阻塞接受客户端的连接请求
        errorputs("connect() failed");
    if (read(clisock, buf, SIZE) == -1) //Linux也有send和recv函数，它们和Windows下的函数几乎一致
        errorputs("recv() failed!");
    printf("Reply from server: %s\n", buf);
    puts("send 1");
    write(clisock, message, sizeof(message));
    puts("send 2");
    // sleep(5);
    // if (write(clisock, message, sizeof(message)) == -1) //与Windows不同，服务端关闭后使用收发函数不会出错
    //     errorputs("write() failed!");

    shutdown(clisock, SHUT_WR);
    if (read(clisock, buf, SIZE) == -1)
        errorputs("recv() failed!");
    printf("Reply from server: %s\n", buf);

    return 0;
}