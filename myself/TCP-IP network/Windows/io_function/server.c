#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
//基于select的服务端，recv函数的参数4可选项实验，将参数4设置为MSG_OOB带外数据模式，当然TCP只是将其处理为紧急数据而已
//在Linux中可以用SIGURG信号处理机制来处理OOB，但Windows没有信号处理，所以使用select机制，OOB属于fd的异常事件
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
    fd_set fd[2], fdtemp[2]; //文件描述符设置结构体，fd[0]为可读，fd[1]为异常，fdtemp对应fd的临时量
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
    if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == SOCKET_ERROR)
        ErrorPuts("accept() failed!");
    send(clisock, reply_1, sizeof(reply_1), 0);

    FD_ZERO(&fd[0]); //初始化
    FD_ZERO(&fd[1]);
    FD_SET(clisock, &fd[0]); //设置服务端的可读和异常fd
    FD_SET(clisock, &fd[1]);

    while (1) //服务端与客户端循环连接的主体
    {
        fdtemp[0] = fd[0];
        fdtemp[1] = fd[1];
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if ((fdnum = select(0, &fdtemp[0], NULL, &fdtemp[1], &timeout)) == -1) //监视客户端的输入和异常
            ErrorPuts("select() failed!");
        else if (fdnum == 0) { //超时
            puts("timeout...");
            continue;
        }
        else { //fd有事件发生
            if (FD_ISSET(clisock, &fdtemp[0])) { //客户端fd可读，读取数据
                if ((len = recv(clisock, message, SIZE, 0)) == -1) { //输入出错，需要清除出错的客户端
                    perror("recv() failed");
                    closesocket(clisock);
                    break;
                }
                else if (len == 0) { //客户端正常关闭
                    send(clisock, reply_2, sizeof(reply_2), 0);
                    closesocket(clisock);
                    printf("close client %llu\n", clisock);
                    break;
                }
                else { //正常读入数据
                    message[len] = 0;
                    printf("message of %d byte: %s\n", len, message);
                }
            }
            if (FD_ISSET(clisock, &fdtemp[1])) { //客户端fd异常，有OOB信息
                if (recv(clisock, message, 1, MSG_OOB) == -1) { //使用MSG_OOB读入紧急信息，注意TCP只能读入末尾字符
                    perror("recv(MSG_OOB) failed");
                    closesocket(clisock);
                    break;
                }
                char c = message[0];
                len = recv(clisock, message, SIZE, 0); //这里再正式读入OOB剩余信息
                message[len] = c; //将末尾字符和后续读入的字符串合并
                message[++len] = 0;
                printf("Urgent message of %d byte: %s\n", len, message);
            }
        }
    }

    closesocket(sersock);
    WSACleanup();
    return 0;
}