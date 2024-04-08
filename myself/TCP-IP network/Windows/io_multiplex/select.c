#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
//基于IO复用的服务端，采用select函数监视客户端的fd，与Linux不同，Windows没有fd只有句柄，句柄是随机的整数，所以fd_set无法采用标志位的形式而是直接记录句柄和总数

#define ErrorPuts(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET sersock, clisock;
    SOCKADDR_IN seraddr, cliaddr;
    int szcliaddr = sizeof(cliaddr);
    char reply_1[] = "connecting";
    char reply_2[] = "over";
    const int SIZE = 1024;
    char *message = (char *)malloc(SIZE);
    int fdnum, len; //select返回值，字符串长度
    fd_set fd, fdtemp; //文件描述符设置结构体
    struct timeval timeout; //超时时间

    if (argc != 2)
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    
    if ((sersock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        ErrorPuts("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR)
        ErrorPuts("bind() failed!");
    if (listen(sersock, 3) == SOCKET_ERROR)
        ErrorPuts("listen() failed!");

    FD_ZERO(&fd); //初始化
    FD_SET(sersock, &fd); //设置服务端的fd
    SOCKET sock; //与Linux不同，查询可读fd时不是采用轮训标志位，fd_set中已经记录了fd的数目，这里的sock为临时复制的fd以方便重复使用

    while (1) //服务端与客户端循环连接的主体
    {
        fdtemp = fd;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if ((fdnum = select(0, &fdtemp, NULL, NULL, &timeout)) == -1) //监视服务端的输入，参数1无意义，仅为了兼容Linux版，参数2到4分别为可读、可写、异常(注意异常不是错误)，参数5设置超时时间
            ErrorPuts("select() failed!");
        else if (fdnum == 0) {
            puts("timeout...");
            continue;
        }
        for (int i = 0; i<fd.fd_count; i++) //查询所有可读的fd
        {
            sock = fd.fd_array[i];
            if (FD_ISSET(sock, &fdtemp)) {
                if (sock == sersock) { //服务器fd可读，说明有客户端请求连接
                    if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == -1) {
                        puts("accept() failed!");
                        continue;
                    }
                    printf("new client %llu\n", clisock);
                    send(clisock, reply_1, sizeof(reply_1), 0);
                    FD_SET(clisock, &fd); //设置客户端的fd
                }
                else { //客户端fd可读，意味着客户端发送了信息
                    if ((len = recv(sock, message, SIZE, 0)) == -1) { //输入出错，需要清除出错的客户端以免死循环
                        printf("client %llu read() failed!\n", sock);
                        closesocket(sock);
                        FD_CLR(sock, &fd);
                        continue;
                    }
                    else if (len == 0) { //客户端正常关闭
                        send(sock, reply_2, sizeof(reply_2), 0);
                        closesocket(sock);
                        FD_CLR(sock, &fd);
                        printf("close client %llu\n", sock);
                    }
                    else {
                        printf("message from client %llu of %d byte: %s\n", sock, len, message);
                    }
                }
            }
        }
    }

    closesocket(sersock);
    WSACleanup();
    return 0;
}