#include <stdio.h>
#include <unistd.h>
//双管道双向传输测试，使用两个管道分别传输两个进程的数据，避免数据竞争，这是最常用的做法
int main()
{
    int fd1[2], fd2[2]; //两个数组对应两个管道
    char str1[] = "Hello, how shit are you?";
    char str2[] = "I'm fin, fuck you!";
    const int SIZE = 1024;
    char buf[SIZE];
    pid_t pid;

    pipe(fd1); //主进程输出用管道
    pipe(fd2); //子进程输出用管道
    pid = fork();
    if (pid == 0) {
        write(fd2[1], str1, sizeof(str1)); //子进程使用自己的输出管道的输出口输出
        usleep(1000);
        read(fd1[0], buf, sizeof(buf)); //子进程使用主进程的输出管道的输入口输入
        printf("Child %d: %s\n", getpid(), buf);
    }
    else {
        read(fd2[0], buf, sizeof(buf));
        printf("Parent %d: %s\n", getpid(), buf);
        write(fd1[1], str2, sizeof(str2));
        usleep(2000);
    }
}