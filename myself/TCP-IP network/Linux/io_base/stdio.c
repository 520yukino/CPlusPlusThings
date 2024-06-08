#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h> //更多的时间组件
#define SIZE 1024

int main()
{
    int fd1, fd2, len;
    char buf[SIZE];
    fd1 = open("/mnt/d/YX/wuppo/Wup1.0.43.exe", O_RDONLY); //注意WSL的路径不一样
    fd2 = open("test", O_WRONLY|O_CREAT|O_TRUNC); //只写、没有就创建、打开后清空
    
    struct timeval tv1, tv2; //储存当前的秒和微秒
    gettimeofday(&tv1, NULL);
    while ((len = read(fd1, buf, SIZE))>0)
        write(fd2, buf, SIZE);
    gettimeofday(&tv2, NULL);
    if (tv2.tv_usec>=tv1.tv_usec) //判断微秒的大小
        printf("duration: %lds, %ldus\n", tv2.tv_sec-tv1.tv_sec, tv2.tv_usec-tv1.tv_usec);
    else
        printf("duration: %lds, %ldus\n", tv2.tv_sec-tv1.tv_sec-1, tv1.tv_usec-tv2.tv_usec);

    close(fd1);
    close(fd2);
}