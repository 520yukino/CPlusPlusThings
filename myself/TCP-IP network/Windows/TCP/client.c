#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define ErrorPuts(s) do { \
    puts(s); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET clisock; //套接字句柄
    SOCKADDR_IN seraddr; //套接字地址信息
    char message[] = "I'm client, Hello World!";
    const int SIZE = 1024;
    char *buf = (char *)malloc(SIZE);

    if (argc != 3) //需要传入IP和端口号
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(3, 2), &wsadata)) //winsock启动
        ErrorPuts("WSAStartup() failed!");
    
    if ((clisock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) //创建客户端套接字
        ErrorPuts("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr)); //设置seraddr
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = inet_addr(args[1]); //通过命令行输入的IP地址字符串来规定服务器端
    seraddr.sin_port = htons(atoi(args[2])); //端口必须与服务器接收时使用的端口一致

    if (connect(clisock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR) //连接请求，如果一段时间无法连接则返回SOCKET_ERROR，注意客户端是无法同时连接多个服务器端的
        ErrorPuts("connect() failed");
    if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //从服务器端接收数据
        ErrorPuts("recv() failed!");
    printf("Reply from server: %s\n", buf);
    puts("Client: send begin");
    srand(time(NULL));
    for (int i = 0; i < (rand()%5+1); i++)
    {
        send(clisock, message, sizeof(message), 0); //通知客户端接收完毕，send不会等待对方接收
        Sleep(200);
    }
    send(clisock, "quit", 5, 0); //仅仅只是为了配合服务端退出接收循环
    puts("Client: send over");
    ///注意客户端的输出流关闭后进行send输出操作会破坏掉CLOSE_WAIT状态，彻底断开连接，进而使得recv和第二次send出错，所以服务器退出接收循环后不要再send
    // if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //从服务器端接收数据
    //     ErrorPuts("recv() failed!");
    // printf("Reply from server: %s\n", buf);

    closesocket(clisock);
    WSACleanup();
    return 0;
}