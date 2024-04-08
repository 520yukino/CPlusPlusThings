#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
//进程的信号处理，包括处理僵尸进程，注意进程处理信号时由于要调用相关处理函数，所以会从sleep中直接唤醒
void F_SignAlarm(int sig)
{
    if (sig == SIGALRM)
        printf("process %d: SIGALRM\n", getpid());
    alarm(1);
}

void F_SignInterrupt(int sig)
{
    if (sig == SIGINT)
        printf("process %d: SIGINT\n", getpid());
    exit(2);
}

void F_SignChild(int sig)
{
    if (sig == SIGCHLD)
        printf("process %d: SIGCHLD\n", getpid());
    int stat;
    pid_t pid = wait(&stat); //等待子进程退出，消灭僵尸子进程
    if (WIFEXITED(stat))
        printf("I am parent %d, my child %d exited and send %d\n", getpid(), pid, WEXITSTATUS(stat));
}

int main()
{
    struct sigaction sa; //sigaction结构，储存信号处理有关设定的3个成员，第1个为信号发生时执行的函数指针，后2个指定选项和特性
    sa.sa_handler = F_SignAlarm; //设置SIGALRM信号对应的响应函数
    sigemptyset(&sa.sa_mask); //第二个参数设空，下面的第三个同理
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL); //调用sigaction函数设置对应信号发生时的处理手段，第3个参数可获取之前注册的sigaction结构
    alarm(1); //设置多少秒后发出SIGALRM

    sa.sa_handler = F_SignInterrupt;
    sigaction(SIGINT, &sa, NULL); //设置按下Ctrl+C中断进程指令时执行的函数，注意此设置会覆盖系统原有的中断操作进而只执行设置的函数内容

    pid_t pid = fork(); //注意fork后面的代码父子进程都会执行，这也意味着如果在后面设置sigaction那么父子进程都会受影响，在命令行按下Ctrl+c会分别向父子进程发送SIGINT，但使用kill则可以指定进程
    sa.sa_handler = F_SignChild;
    sigaction(SIGCHLD, &sa, NULL); //设置子进程退出时执行的函数
    if (pid == 0)
    {
        printf("I'm child %d\n", getpid());
        sleep(5);
        return 1;
    }
    else
    {
        printf("I'm parent %d, my child is %d\n", getpid(), pid);
        for (int i = 1; i<=10; i++)
        {
            printf("sleep %d\n", i);
            sleep(1);
        }
    }

}