#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <pthread.h>
//全局聊天的服务端，接收客户端信息后发送至所有客户端

#define ErrorPuts(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

#define CLI_MAX 256 //客户端数目
#define CLI_NAME 20 //客户端id
#define BUF_SIZE 1024 //信息大小

void *TdClient(void *); //每个客户端连接后创键的线程

pthread_mutex_t mutex; //锁住临界区，也就是下方的cli全局变量
SOCKET cliarr[CLI_MAX]; //客户端群组和其数量
int clicount = 0;

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET sersock, clisock;
    SOCKADDR_IN seraddr, cliaddr;
    int szcliaddr = sizeof(cliaddr);
    pthread_t pth;
    char name[CLI_NAME];

    if (argc != 2) {
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
    pthread_mutex_init(&mutex, NULL);

    while (1) //循环创建客户端连接以及客户端线程
    {
        if ((clisock = accept(sersock, (SOCKADDR *)(&cliaddr), &szcliaddr)) == INVALID_SOCKET)
            ErrorPuts("accept() failed!");
        pthread_mutex_lock(&mutex);
        cliarr[clicount++] = clisock;
        pthread_mutex_unlock(&mutex);
        pthread_create(&pth, NULL, TdClient, (void *)&clisock);
        pthread_detach(pth);
        recv(clisock, name, CLI_NAME, 0);
        printf("Server: client %d connect, id: %s, ip: %s - there are %d client(s)\n", clisock, name, inet_ntoa(cliaddr.sin_addr), clicount);
    }

    closesocket(sersock);
    WSACleanup();
    return 0;
}

void *TdClient(void *arg)
{
    SOCKET sock = *(SOCKET *)arg; //客户端的套接字
    int len;
    char msg[BUF_SIZE];

    while (1) //循环接收客户端信息并响应
    {
        len = recv(sock, msg, BUF_SIZE, 0);
        if (len > 0) { //正常，发送信息至所有客户端
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < clicount; i++)
            {
                send(cliarr[i], msg, len, 0);
            }
            pthread_mutex_unlock(&mutex);
        }
        else { //关闭或异常，清除客户端相关内存
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < clicount; i++)
            {
                if (cliarr[i] == sock) {
                    cliarr[i] = cliarr[--clicount];
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            if (!len) //客户端正常关闭
                printf("Server: client %d closed\n", sock);
            else //recv错误，发送错误信息
                printf("recv() error! WSAErrorCode: %d\n", WSAGetLastError());
            closesocket(sock); //拿到GetError的后方，因为调用套接字相关函数后会重置error码
            return NULL;
        }
    }
}