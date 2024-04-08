#include <stdio.h>
#include <unistd.h>
#include <sys/uio.h>
//readv和writev函数，它们可以批量读写数据，使用iovec结构数组储存读写的数据和长度，
int main()
{
    struct iovec iov[2];
    char str1[] = "ABC";
    char str2[] = "12345";
    char buf1[5] = {}, buf2[100] = {};
    int len;

    iov[0].iov_base = str1; //需要输出的数据
    iov[0].iov_len = sizeof(str1)-1; //输出数据的长度
    iov[1].iov_base = str2;
    iov[1].iov_len = sizeof(str2)-1;
    len = writev(1, iov, 2); //参数1为fd，参数2和3为iovec数组及其元素个数
    printf("\nWritev byte: %d\n", len);

    iov[0].iov_base = buf1; //需要输入的数据
    iov[0].iov_len = sizeof(buf1)-1; //输入数据的长度
    iov[1].iov_base = buf2;
    iov[1].iov_len = sizeof(buf2)-1;
    len = readv(0, iov, 2); //注意readv是读取数据并依次装在用户提供的buf中，总是装满前一个在装后一个，特殊的是stdin中的换行符，它会被当做直接完成一次读入，而一般文本中的\n则只是一个字符，但一个例外是前一个buf刚好读满(包括\n)，这时会继续读下一个buf并重复前述行为
    printf("Readv byte: %d\n", len);
    printf("buf 1: %s, buf 2: %s\n", buf1, buf2);
}