#include <unistd.h>
#include <stdio.h>
//需在linux下编译运行
int main(void)
{
    int i;
    pid_t pid;
    for (i = 0; i < 3; ++i) {
        pid = fork();
        // if (pid == 0) //子线程自身返回值为0，父线程则为子线程pid
        // {
        //     break;
        // }
    }

    if (pid == 0) {
        sleep(3); //如果主进程先于子进程退出，则子进程会交于init原初进程管理，即getppid为1
        printf("I am child: %d, my parent: %d\n", getpid(), getppid());
    }
    else {
        sleep(1);
        printf("I am parent: %d\n", getpid());
    }

    return 0;
}