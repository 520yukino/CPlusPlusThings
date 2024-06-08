#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
/* 输入版回声客户端，读入用户输入并将信息发送至服务端，可以识别关键词来退出输入 */

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

    if (connect(clisock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR) //连接请求
        errorputs("connect() failed");
    puts("Client: send begin");
    while (1)
    {
        fputs("Input: ", stdout);
        fgets(message, SIZE, stdin);
        if (!strncmp(message, "exit", 4) || !strncmp(message, "quit", 4)) //关键词退出
            break;
        send(clisock, message, strlen(message)+1, 0);
        if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //从服务器端接收回声数据
            errorputs("recv() failed!");
        fprintf(stdout, "Echo: %s", buf);
    }
    puts("Client: send over");

    closesocket(clisock);
    WSACleanup();
    return 0;
}