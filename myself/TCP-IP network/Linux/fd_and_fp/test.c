#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
//使用Linux的文件io和标准io都需要规定其参数，都具有读写的区分性质，并且fdopen函数的转换必须保持该文件的读写权限一致
int main()
{
    int fd1, fd2;
    FILE *fp1, *fp2;
    char msg[] = "欧金金";
    fd1 = open("something.txt", O_RDWR);
    printf("%d\n", fd1);
    fp1 = fdopen(fd1, "a+"); //fdopen只是将fd转为fp，并没有使用fopen打开文件时的处理，所以w不会清空文件，但a依旧会使指针移到文件尾
    printf("%p, %d\n", fp1, fileno(fp1)); //fileno可以将fp变为fd

    fd2 = dup(fd1); //复制fd，复制后必须删除全部fd才能彻底关闭文件，不过使用shutdown依旧可以关闭给定流
    //注意，在套接字编程中，可以使用shutdown(fd1, SHUT_WR);来直接关闭套接字的输入输出流，即使该套接字有多个fd。但shutdown只用于套接字而非文件
    write(fd1, msg, sizeof(msg));
    close(fd2);
    write(fd1, msg, sizeof(msg));
    close(fd1);
    write(fd1, msg, sizeof(msg)); //已完全关闭
}