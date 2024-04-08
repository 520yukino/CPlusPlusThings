#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
//使用对子进程的信号处理技术实现并发服务器

#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

void F_SignChild(int sig)
{
    int stat;
    pid_t pid = wait(&stat); //等待子进程退出，消灭僵尸子进程
    if (WIFEXITED(stat))
        printf("%d child %d exited and send %d\n", getpid(), pid, WEXITSTATUS(stat));
}

void F_SignInterrupt(int sig) //即使这里的信号处理函数覆盖了系统默认的，进程中断时依旧会正常close套接字
{
    if (sig == SIGINT)
        printf("process %d: SIGINT\n", getpid());
    fflush(stdout);
    exit(2);
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

    if (argc != 2)
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    struct sigaction sa; //设置子进程退出时的信号处理函数和中断时的函数
    sa.sa_handler = F_SignChild;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);
    sa.sa_handler = F_SignInterrupt;
    sigaction(SIGINT, &sa, NULL);

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
    
    while (1) //循环连接客户端并托管到子进程，注意子进程也会复制套接字，需要关闭其中多余的套接字以免套接字耗尽，主进程管理服务端，子进程管理客户端
    {
        if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == -1) { //断开连接后的第一次accept会出错，第二次才正常阻塞等待连接请求，待探究。。。
            // #include <errno.h>    
            // printf("accept error: %d\n", errno);
            continue; //某一次连接失败并不意味着需要中断主进程，也可以放弃此次连接，毕竟还有之前创建的连接和子进程
        }
        else
            puts("new client...");
        if ((pid = fork()) == -1) { //创建子进程失败，只能放弃这一次连接，因为客户端连接依附于子进程
            puts("fork() fail!");
        }
        else if (pid == 0) { //子进程，沟通客户端
            close(sersock); //子进程中多余的服务端套接字需要关闭
            write(clisock, reply_1, sizeof(reply_1)); //通知客户端已联通
            if ((res = read(clisock, message, SIZE)) == -1)
                errorputs("recv() failed!");
            printf("I'm child %d, %d %d, message from client of %d byte(s): %s\n", getpid(), clisock, sersock, res, message);
            write(clisock, reply_2, sizeof(reply_2)); //通知客户端接收完毕
            close(clisock); //子进程需要关闭客户端套接字
            sleep(2);
            puts("client disconnected...");
            return 1; //子进程退出
        }
        close(clisock); //无论主进程fork成功与否，最终都需要关闭客户端套接字，注意主进程不关闭套接字会使得这个套接字被占用
    }

    close(sersock);
    return 0;
}