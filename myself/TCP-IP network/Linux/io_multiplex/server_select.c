#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
//基于IO复用的服务端，采用select函数监视客户端的fd

#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char* args[])
{
    int sersock, clisock;
    struct sockaddr_in seraddr, cliaddr;
    socklen_t szcliaddr = sizeof(cliaddr);
    char reply_1[] = "connecting";
    char reply_2[] = "over";
    const int SIZE = 1024;
    char* message = (char*)malloc(SIZE);
    int fdnum, fdmax, len; //select返回值，fd的最大值，字符串长度
    fd_set fd, fdtemp; //文件描述符设置结构体
    struct timeval timeout; //超时时间

    if (argc != 2)
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if ((sersock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (struct sockaddr*)(&seraddr), sizeof(seraddr)) == -1)
        errorputs("bind() failed!");
    if (listen(sersock, 5) == -1)
        errorputs("listen() failed!");

    FD_ZERO(&fd); //初始化
    FD_SET(sersock, &fd); //将服务端fd置1
    fdmax = sersock; //记录当前fd最大值
    
    while (1) //服务端与客户端循环连接的主体
    {
        fdtemp = fd; //使用临时的fd
        timeout.tv_sec = 5; //设置时间
        timeout.tv_usec = 0;
        if ((fdnum = select(fdmax+1, &fdtemp, NULL, NULL, &timeout)) == -1) //监视服务端的输入，因为客户端的连接请求本就需要发送信息，所以将select放在accept前面
            errorputs("select() failed!");
        else if (fdnum == 0) {
            puts("timeout...");
            continue;
        }

        for (int i = 0; i <= fdmax; i++) //查询所有可读的fd
        {
            if (FD_ISSET(i, &fdtemp)) {
                if (i == sersock) { //服务器fd可读，说明有客户端请求连接
                    if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == -1) {
                        puts("accept() failed!");
                        continue;
                    }
                    printf("new client %d\n", clisock);
                    write(clisock, reply_1, sizeof(reply_1));
                    FD_SET(clisock, &fd); //设置客户端的fd
                    if (fdmax<clisock) //更新fdmax
                        fdmax = clisock;
                }
                else { //客户端fd可读，意味着客户端发送了信息
                    if ((len = read(i, message, SIZE)) == -1) { //输入出错，需要清除出错的客户端以免死循环
                        printf("client %d read() failed!\n", i);
                        close(i);
                        FD_CLR(i, &fd);
                        continue;
                    }
                    else if (len == 0) { //客户端关闭
                        write(i, reply_2, sizeof(reply_2));
                        close(i);
                        FD_CLR(i, &fd);
                        printf("close client %d\n", i);
                    }
                    else {
                        printf("message from client %d of %d byte: %s\n", i, len, message);
                    }
                }
            }
        }
    }

    close(sersock);
    return 0;
}