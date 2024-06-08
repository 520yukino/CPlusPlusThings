#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <pthread.h>
//全局聊天的客户端，发送的信息会被所有在线用户接收

#define ErrorPuts(s) do { \
    puts(s); \
    exit(-2); \
} while(0)

void *TdRecvMsg(void *arg); //接收信息
void *TdSendMsg(void *arg); //发送信息

#define BUF_SIZE 1024 //信息大小
#define NAME_SIZE 20 //id大小
#define MSG_RECV_TIMES 100 //接收信息的次数
#define MSG_SEND_TIMES 10 //发送信息的次数
char name[NAME_SIZE] = "[-NONE-]"; //用户id，统一用中括号括起

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET clisock;
    SOCKADDR_IN seraddr;
    pthread_t pth[2]; //io分离所用线程

    if (argc != 4) //额外需要输入用户id
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(3, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    
    if ((clisock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        ErrorPuts("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = inet_addr(args[1]);
    seraddr.sin_port = htons(atoi(args[2]));
    if (connect(clisock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR)
        ErrorPuts("connect() failed!");

    if (strlen(args[3])>NAME_SIZE-3) //截取合理长度的id并发给server，中括号+结尾空字符需要占用3个字节
        args[3][NAME_SIZE-3] = 0;
    sprintf(name, "[%s]", args[3]);
    send(clisock, name, NAME_SIZE, 0);

    pthread_create(pth, NULL, TdRecvMsg, (void *)&clisock);
    pthread_create(pth+1, NULL, TdSendMsg, (void *)&clisock);
    pthread_join(pth[0], NULL);
    pthread_join(pth[1], NULL);

    closesocket(clisock);
    WSACleanup();
    return 0;
}

void *TdRecvMsg(void *arg)
{
    SOCKET sock = *(SOCKET *)arg;
    char buf[NAME_SIZE+BUF_SIZE];
    int len;
    for (int i = 0; i < MSG_RECV_TIMES; i++) //接收信息并显示
    {
        if ((len = recv(sock, buf, sizeof(buf)-1, 0)) == SOCKET_ERROR)
            ErrorPuts("recv() error!");
        buf[len] = '\0';
        puts(buf);
    }
    return NULL;
}

void *TdSendMsg(void *arg)
{
    SOCKET sock = *(SOCKET *)arg;
    char buf[NAME_SIZE+BUF_SIZE];
    int len;
    srand(time(NULL)); //随机间隔时间发送信息
    for (int i = 1; i <= MSG_SEND_TIMES; i++) //结合id一并发送
    {
        sprintf(buf, "%s: message %d, %d", name, i, rand());
        send(sock, buf, strlen(buf), 0);
        Sleep(rand()%2000);
    }
    return NULL;
}