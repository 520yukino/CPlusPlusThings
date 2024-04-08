#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
//IO复用，Linux中的select与Windows中的有所不同，它的fd_set结构记录的是1024个文件描述符标志位(Linux中文件描述符最大为1023，从0开始)，需要谁就置1
int main()
{
    fd_set frd, fwr, ftemp[2]; //设置需要监视的文件描述符时所需要的结构变量，frd和fwr为读和写的fd，ftemp作为临时fd在select函数参数中很重要
    int res, len;
    const int SIZE = 1024;
    char buf[SIZE];
    struct timeval timeout; //设置监视的超时时间

    FD_ZERO(&frd); //所有位置0
    FD_SET(0, &frd); //将0号描述符置1，这是标准输入
    FD_ZERO(&fwr);
    FD_SET(1, &fwr);

    while (1)
    {
        ftemp[0] = frd; //select会更改传入的设置描述符的结构变量，所有每次复用都要重新赋值
        ftemp[1] = fwr;
        timeout.tv_sec = 3; //每次复用都要设置时间，因为调用select后timeout将记录超时前的剩余时间
        timeout.tv_usec = 0;
        res = select(1, &ftemp[0], NULL, NULL, &timeout);
        // res = select(2, &ftemp[0], &ftemp[1], NULL, &timeout);
        /*
        io复用的主体函数，参数1是需要查看的最大fd，设置为三组描述符中的最大描述符+1，因为描述符从0开始，这个参数使函数不必检查fd_set的所有1024位
        参数2到4都是fd_set *，它们分别表示需要监视的输入fd、输出、异常(但不包括错误)，没有就传0，当监视到传入的描述符中有对应事件时就保留这个描述符对应的标志位，其它没有事件的则置0
        参数5设置超时时间，用法如下：1. timeout=NULL(阻塞：直到有一个fd位被置为1函数才返回) 2. timeout所指向的结构设为非零时间(等待：有一个fd位被置为1或者时间耗尽，函数均返回) 3. timeout所指向的结构，时间设为0(非阻塞：函数轮训检查完需要监视的fd后立即返回)
        函数返回值为-1为出错，0为超时，正数为监视到发送了事件的fd个数
        就本例而言，可读意味着需要有用户输入到输入缓冲区，而可写却是一直存在，可读写的本质是读写不阻塞，所以监视了输出fd的select永远不会超时，因为始终可写
        */
        if (res == -1) {
            puts("select error!");
            return -1;
        }
        else if (res == 0) {
            puts("select timeout...");
        }
        else {
            if (FD_ISSET(0, &frd)) { //如果stdin在frd中的标志为1则说明有数据可以读入
                len = read(0, buf, SIZE-1);
                buf[len] = 0;
                printf("message: %s", buf);
            }
        }
    }
}