#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define errorputs(s) { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
}

int main(int argc, char* args[])
{
    int sersock, clisock; //套接字句柄
    struct sockaddr_in seraddr, cliaddr; //套接字地址信息
    socklen_t szcliaddr = sizeof(cliaddr);
    const int SIZE = 1024;
    char* message = (char*)malloc(SIZE);
    char reply_1[] = "connecting";
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
    if (listen(sersock, 10) == -1) //转为可接收状态
        errorputs("listen() failed!");

    int n = 0; //连接次数
    FILE *file = fopen("log.txt", "w");
    while (1) //压力测试，循环连接
    {
        if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == -1)
            errorputs("accept() failed!");
        // write(clisock, reply_1, sizeof(reply_1));
        // puts("send reply_1");
        if ((res = read(clisock, message, SIZE)) == -1)
            errorputs("recv() failed!");
        fprintf(file, "Message from client %d of %d byte(s): %s\n", ++n, res, message);
        fflush(file); //由于服务端使用Ctrl+C结束运行，所以需要不断刷新缓冲区以免最终输出的文件内容不完整
        // puts("send reply_2");
        // write(clisock, reply_2, sizeof(reply_2));
        close(clisock);
    }

    fclose(file);
    close(sersock);
    return 0;
}