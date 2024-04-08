#include <stdio.h>
#include <unistd.h>
//进程间通讯的单管道双向传输测试，管道是内核资源而非进程资源，不会被子进程复制。注意管道一般只用于单相传输，双向传输会有隐患
int main()
{
    int fd[2]; //文件描述符数组
    char str1[] = "Hello, how shit are you?";
    char str2[] = "I'm fin, fuck you!";
    const int SIZE = 1024;
    char buf[SIZE];
    pid_t pid;

    pipe(fd); //创建管道并分配fd，0号元素为文件的输入，管道的输出；1号则相反
    pid = fork(); //关键在于pipe后再fork，这样主进程和子进程都能访问fd
    if (pid == 0) { //双向使用管道
        write(fd[1], str1, sizeof(str1));
        usleep(1000); //单管道双向传输的最大弊端在于管道的数据输入输出只依附于fd，所以进程的输出可以被任意进程包括自己读入，这会导致需要读入这段数据的进程无法读入进而导致read阻塞混乱，此处采用等待1ms的方式确保另一个进程先读
        read(fd[0], buf, sizeof(buf));
        printf("Child %d: %s\n", getpid(), buf);
    }
    else {
        read(fd[0], buf, sizeof(buf));
        printf("Parent %d: %s\n", getpid(), buf);
        write(fd[1], str2, sizeof(str2));
        usleep(2000); //主进程应该等待子进程结束后再结束，以免终端输出混乱
    }
}