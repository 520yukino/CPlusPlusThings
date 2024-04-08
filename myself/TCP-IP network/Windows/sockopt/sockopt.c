#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <in6addr.h>

#define ErrorPuts(s) { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
}
void ShowSocketBufferSize(SOCKET sock);

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET tcpsock, udpsock;
    int tcpopt, udpopt, len = sizeof(tcpopt);
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    if ((tcpsock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        ErrorPuts("socket() 1 failed!");
    if ((udpsock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        ErrorPuts("socket() 2 failed!");

    //getsockopt函数是获取某一套接字的选项，SOL_SOCKET为套接字选项组，IPPROTO_IP和IPPROTO_TCP是IP层和传输层的选项组，其中SO_TYPE为套接字选项组中的套接字连接类型
    if (getsockopt(tcpsock, SOL_SOCKET, SO_TYPE, (char *)&tcpopt, &len))
        ErrorPuts("getsockopt() 1 error!");
    if (getsockopt(udpsock, SOL_SOCKET, SO_TYPE, (char *)&udpopt, &len))
        ErrorPuts("getsockopt() 2 error!");
    printf("SOCK_STREAM = %d, SOCK_DGRAM = %d\n", SOCK_STREAM, SOCK_DGRAM);
    printf("TCP SO_TYPE = %d, UDP SO_TYPE = %d\n", tcpopt, udpopt);

    //setsockopt函数是设置某一套接字的选项，此处SO_SNDBUF和SO_RCVBUF为输出缓冲区大小和输入缓冲区大小，注意设置后并不意味着一定为这个大小，系统自有决断
    tcpopt = 0, udpopt = 1000000000;
    puts("TCP socket:");
    ShowSocketBufferSize(tcpsock);
    ShowSocketBufferSize(udpsock);
    if (setsockopt(tcpsock, SOL_SOCKET, SO_SNDBUF, (char *)&tcpopt, len))
        ErrorPuts("setsockopt() 1 error!");
    if (setsockopt(udpsock, SOL_SOCKET, SO_RCVBUF, (char *)&udpopt, len))
        ErrorPuts("setsockopt() 2 error!");
    puts("UDP socket:");
    ShowSocketBufferSize(tcpsock);
    ShowSocketBufferSize(udpsock);

    //SO_REUSEADDR为是否允许直接复用处于TIME_WAIT状态的端口，默认为0不可复用，但实验表明可以复用，待探究
    if (getsockopt(tcpsock, SOL_SOCKET, SO_REUSEADDR, (char *)&tcpopt, &len))
        ErrorPuts("getsockopt() 3 error!");
    printf("TCP SO_REUSEADDR = %d\n", tcpopt);
    //IPPROTO_TCP选项组中的TCP_NODELAY为是否禁用Nagle算法，默认为0不禁用，此算法是指等待收到ack回应信号后再传输下一个数据而非直接传输，可以有效减少服务器过多发送数据包导致的网络过载，但在大文件传输时会明显减缓传输速度，所以需要酌情禁用
    if (getsockopt(tcpsock, IPPROTO_TCP, TCP_NODELAY, (char *)&tcpopt, &len))
        ErrorPuts("getsockopt() 4 error!");
    printf("TCP TCP_NODELAY = %d\n", tcpopt);

    WSACleanup();
}

void ShowSocketBufferSize(SOCKET sock) //显示某套接字的输入输出缓冲区大小
{
    int sndopt, rcvopt, len = sizeof(sndopt);
    if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&sndopt, &len))
        ErrorPuts("ShowSocketBufferSize() 1 error!");
    if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&rcvopt, &len))
        ErrorPuts("ShowSocketBufferSize() 2 error!");
    printf("SO_SNDBUF = %d, SO_RCVBUF = %d\n", sndopt, rcvopt);
}