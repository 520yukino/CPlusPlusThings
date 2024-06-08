#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h> //用于struct ip_mreq
//receiver需要等待接收多播信息，所以需要bind绑定网络端口
#define ErrorPuts(s) do { \
    puts(s); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET recvsock; //接收者
    SOCKADDR_IN recvaddr; //接收者地址信息
    IP_MREQ joinaddr;
    const int SIZE = 1024+1;
    char *buf = (char *)malloc(SIZE);
    int res;

    if (argc != 3) //需要传入多播IP和端口号
    {
        printf("Usage: %s: <GroupIP> <Port>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    
    if ((recvsock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) //多播采用UDP传输方式
        ErrorPuts("socket() failed!");
    memset(&recvaddr, 0, sizeof(recvaddr)); //设置自身地址信息并绑定，以此创建UDP网络成为接收者
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    recvaddr.sin_port = htons(atoi(args[2])); //注意多播必须保证发送者和接收者的端口号一致
    if (bind(recvsock, (SOCKADDR *)(&recvaddr), sizeof(recvaddr)) == SOCKET_ERROR)
        ErrorPuts("bind() failed!");
    
    //设置接收者加入多播组，需要加入的多播地址和自身地址，而后设置IP层的套接字选项
    joinaddr.imr_multiaddr.S_un.S_addr = inet_addr(args[1]);
    joinaddr.imr_interface.S_un.S_addr = htonl(INADDR_ANY);
    if (setsockopt(recvsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)(&joinaddr), sizeof(joinaddr)) == SOCKET_ERROR)
        ErrorPuts("setsockopt() failed!");

    puts("Receiver: recv begin");
    while (1) {
        res = recv(recvsock, buf, SIZE-1, 0); //由于UDP是有数据边界的，所以recv的大小需要大于send的大小，否则recv会截断数据报并返回-1
        if (res == -1) {
            fprintf(stderr, "WSAError: %d\n", WSAGetLastError());
            continue;
        }
        buf[res] = '\0';
        printf("recv %d byte(s): %s\n", res, buf);
    }
    puts("Receiver: recv over");
   
    closesocket(recvsock);
    WSACleanup();
    return 0;
}