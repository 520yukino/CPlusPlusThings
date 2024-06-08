#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
//readv和writev函数，它们可以批量读写数据，使用iovec结构数组储存读写的数据和长度
int main()
{
    struct iovec iov[2]; //将iovec结构初始化后传入v系列io函数即可使用
    char *str1 = "ABC";
    char *str2 = "12345";
    char buf1[5] = {}, buf2[100] = {};
    int len;

    iov[0].iov_base = str1; //需要输出的数据
    iov[0].iov_len = strlen(str1); //输出数据的长度
    iov[1].iov_base = str2;
    iov[1].iov_len = strlen(str2);
    len = writev(1, iov, 2); //参数1为fd，参数2和3为iovec数组及其元素个数
    printf("\nWritev byte: %d\n", len);

    iov[0].iov_base = buf1; //输入缓冲区
    iov[0].iov_len = sizeof(buf1)-1; //输入缓冲区大小，需要留出空字符结尾
    iov[1].iov_base = buf2;
    iov[1].iov_len = sizeof(buf2)-1;
    len = readv(0, iov, 2); //注意readv是读取数据并依次装在用户提供的buf中，总是装满前一个在装后一个，特殊的是stdin中的换行符，它会直接结束此函数。而一般文本中的\n则只是一个字符，但一个例外是前一个buf刚好读满(包括\n)，这时会继续读下一个buf并重复前述行为
    printf("Readv byte: %d\n", len);
    printf("buf 1: %s, buf 2: %s\n", buf1, buf2);
}