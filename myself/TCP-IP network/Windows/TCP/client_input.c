#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define errorputs(s) do { \
    puts(s); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET clisock; //套接字句柄
    SOCKADDR_IN seraddr; //套接字地址信息
    const int SIZE = 1024;
    char message[SIZE];
    char *buf = (char *)malloc(SIZE);

    if (argc != 3) //需要传入IP和端口号
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(3, 2), &wsadata)) //winsock启动
        errorputs("WSAStartup() failed!");
    if ((clisock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) //创建服务器端套接字
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr)); //设置seraddr
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = inet_addr(args[1]); //通过命令行输入的IP地址字符串来规定服务器端
    seraddr.sin_port = htons(atoi(args[2])); //端口必须与服务器接收时使用的端口一致

    puts("test_1");
    if (connect(clisock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR) //连接请求
        errorputs("connect() failed");
    puts("test_2");
    if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //从服务器端接收数据
        errorputs("recv() failed!");
    printf("Reply from server: %s\n", buf);
    puts("Client: send begin");
    while (1)
    {
        fgets(message, SIZE, stdin);
        send(clisock, message, sizeof(message), 0); //通知客户端接收完毕，send不会等待对方接收
        if (!strncmp(message, "exit", 4) || !strncmp(message, "quit", 4))
            break;
    }
    send(clisock, "exit", 5, 0);
    puts("Client: send over");

    if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //从服务器端接收数据
    {
        printf("error: %d\n", WSAGetLastError());
        errorputs("recv() failed!");
    }
    printf("Reply from server: %s\n", buf);

    closesocket(clisock);
    WSACleanup();
    return 0;
}