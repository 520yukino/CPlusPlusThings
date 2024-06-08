#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/* 服务端基本模型 */
#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char* args[])
{
    int sersock, clisock; //套接字句柄
    struct sockaddr_in seraddr, cliaddr; //套接字地址信息
    socklen_t szcliaddr = sizeof(cliaddr);
    char reply_1[] = "connecting";
    char reply_2[] = "over";
    const int SIZE = 1024;
    char* message = (char*)malloc(SIZE);
    int res;

    if (argc != 2) //需要传入端口号
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    
    if ((sersock = socket(PF_INET, SOCK_STREAM, 0)) == -1) //创建服务器端套接字
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr)); //设置seraddr
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (struct sockaddr*)(&seraddr), sizeof(seraddr)) == -1) //绑定套接字地址信息
        errorputs("bind() failed!");
    if (listen(sersock, 0) == -1) //转为可接收状态
        errorputs("listen() failed!");
    if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == -1)
        errorputs("accept() failed!");

    puts("send reply_1");
    write(clisock, reply_1, sizeof(reply_1)); //通知客户端已联通
    if ((res = read(clisock, message, SIZE)) == -1)
        errorputs("recv() failed!");
    
    printf("Message from client of %d byte(s): %s\n", res, message);
    puts("send reply_2");
    write(clisock, reply_2, sizeof(reply_2)); //通知客户端接收完毕

    sleep(1);
    // shutdown(clisock, SHUT_RD); //shutdown可以关闭指定的输入输出流，这里关闭了服务器端的输入流(也是客户端的输出流)，注意第一个参数仍然是客户端套接字
    close(clisock);
    close(sersock);
    return 0;
}