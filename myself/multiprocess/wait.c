#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
//需在linux下编译运行
//子进程不会随主进程退出而退出，但如果子进程先退出，那么子进程不会销毁，而是变为僵尸进程，等待主进程退出或使用wait主动销毁
int main(void)
{
    pid_t pid1, pid2;
    int stat;

    pid1 = fork();
    if (pid1 == 0)
    {
        printf("I am child %d, my parent %d\n", getpid(), getppid());
        exit(1);
    }
    else
    {
        pid1 = wait(&stat); //阻塞等待子进程退出，返回pid，失败为-1，传参接收子进程的信息
        if (WIFEXITED(stat)) //宏，为真则子进程正常退出，下面的WEXITSTATUS为子进程返回值
            printf("I am parent %d, my child %d send %d\n", getpid(), pid1, WEXITSTATUS(stat));
    }

    pid2 = fork();
    if (pid2 == 0)
    {
        sleep(1);
        printf("I am child %d, my parent %d\n", getpid(), getppid());
        exit(2);
    }
    else
    {
        //可选式等待子进程退出，返回pid，失败为-1，没有子进程退出则返回0，参数1规定想要等待的子进程pid，-1为等待任意子进程，参数2接收子进程的信息，参数3规定选项，WNOHANG为不阻塞式
        while (!(pid2 = waitpid(-1, &stat, WNOHANG))){
            usleep(300000);
            puts("usleep(300)");
        }
        if (WIFEXITED(stat)) //宏，为真则子进程正常退出，下面的WEXITSTATUS为子进程返回值
            printf("I am parent %d, my child %d send %d\n", getpid(), pid2, WEXITSTATUS(stat));
    }

    return 0;
}