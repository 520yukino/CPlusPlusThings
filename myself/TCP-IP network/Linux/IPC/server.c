#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
//根据multipro_server.c修改而来的基于IPC的服务端，可以将客户端的内容同步输出到文件内

#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

void F_SignChild(int sig)
{
    int stat;
    pid_t pid = wait(&stat);
    if (WIFEXITED(stat))
        printf("%d child %d exited and send %d\n", getpid(), pid, WEXITSTATUS(stat));
}

int main(int argc, char* args[])
{
    int sersock, clisock;
    struct sockaddr_in seraddr, cliaddr;
    socklen_t szcliaddr = sizeof(cliaddr);
    char reply_1[] = "connecting";
    char reply_2[] = "over";
    const int SIZE = 1024;
    char* message = (char*)malloc(SIZE);
    int res;
    pid_t pid;
    int fd[2]; //管道使用的两个文件描述符

    if (argc != 2)
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    struct sigaction sa;
    sa.sa_handler = F_SignChild;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

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
    
    pipe(fd);
    pid = fork();
    if (pid == 0) //子进程，将客户端内容记录到文件
    {
        FILE *file = fopen("message.txt", "wt");
        char fmsgbuf[SIZE];
        int len;
        while (1) { //读取负责与客户端沟通的子进程传来的数据并储存到文件
            len = read(fd[0], fmsgbuf, SIZE);
            fmsgbuf[len-1] = '\n';
            fwrite(fmsgbuf, len, 1, file);
            fflush(file);
        }
        fclose(file);
        return 1; //第一个子进程退出
    }
    
    while (1) //服务端与客户端循环连接的主体
    {
        if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == -1) {
            continue;
        }
        else
            puts("new client...");
        if ((pid = fork()) == -1) {
            puts("fork() fail!");
        }
        else if (pid == 0) { //子进程，沟通客户端
            close(sersock);
            write(clisock, reply_1, sizeof(reply_1));
            if ((res = read(clisock, message, SIZE)) == -1)
                errorputs("recv() failed!");
            printf("I'm child %d, %d %d, message from client of %d byte(s): %s\n", getpid(), clisock, sersock, res, message);
            write(fd[1], message, res); //将客户端传来的内容输出给负责文件储存的子进程
            write(clisock, reply_2, sizeof(reply_2));
            close(clisock);
            sleep(2);
            puts("client disconnected...");
            return 2; //第二个子进程退出
        }
        close(clisock);
    }

    close(sersock);
    return 0;
}