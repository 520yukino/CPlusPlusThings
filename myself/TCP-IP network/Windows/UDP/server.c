#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define ErrorPuts(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET sersock; //只需要服务器套接字，因为UDP不进行连接
    SOCKADDR_IN seraddr, cliaddr; //套接字地址信息
    int szcliaddr = sizeof(cliaddr);
    char reply[] = "receive over";
    const int SIZE = 1024;
    char *message = (char *)malloc(SIZE);

    if (argc != 2) //需要传入端口号
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    
    if ((sersock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) //使用面向数据的传输方式，即UDP
        ErrorPuts("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR) //绑定服务器端地址信息，此函数会占用本机端口并将该程序变为服务端，此后可以阻塞接收信息，客户端也会知晓服务端已绑定
        ErrorPuts("bind() failed!");

    const int SERVER_COUNT = 5; //最大服务器连接次数
    int res; //信息数，recv的返回值
    puts("Server: receive begin");
    for (int i = 1; i <= SERVER_COUNT; i++) //循环服务接收数据并回复
    {
        res = recvfrom(sersock, message, SIZE, 0, (SOCKADDR *)(&cliaddr), &szcliaddr); //通知客户端已联通，注意发送数据不会等待对方的接收，而是直接发送至自身的输出缓冲区后就返回，之后输出缓冲区会发送至对方的输入缓冲区中等待对方recv读入
        if (res == SOCKET_ERROR)
        {
            ErrorPuts("recv() failed!");
            break;
        }
        printf("Message %d of %d byte(s): %s\n", i, res, message);
        sendto(sersock, reply, sizeof(reply), 0, (SOCKADDR *)(&cliaddr), szcliaddr); //通知客户端接收完毕
    }
    printf("Server: receive over\n");

    closesocket(sersock);
    WSACleanup();
    return 0;
}