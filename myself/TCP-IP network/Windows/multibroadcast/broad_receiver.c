#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
//receiver需要等待接收广播信息，所以需要bind绑定网络端口，广播的接收者无需设置选项，因为信息的发送范围已由发送者指定的网络ID所确定
#define ErrorPuts(s) do { \
    puts(s); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET recvsock; //接收者
    SOCKADDR_IN recvaddr; //接收者地址信息
    const int SIZE = 1024+1;
    char *buf = (char *)malloc(SIZE);
    int res;

    if (argc != 2) //需要传入端口号，广播是固定的地址范围，所以无需指定IP
    {
        printf("Usage: %s: <Port>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    
    if ((recvsock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) //多播采用UDP传输方式
        ErrorPuts("socket() failed!");
    memset(&recvaddr, 0, sizeof(recvaddr)); //设置自身地址信息并绑定，以此创建UDP网络成为接收者
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    recvaddr.sin_port = htons(atoi(args[1])); //注意广播必须保证发送者和接收者的端口号一致
    if (bind(recvsock, (SOCKADDR *)(&recvaddr), sizeof(recvaddr)) == SOCKET_ERROR)
        ErrorPuts("bind() failed!");
    
    puts("Receiver: recv begin");
    while (1) {
        res = recv(recvsock, buf, SIZE-1, 0); //由于UDP是有数据边界的，所以recv的大小需要大于send的大小，否则recv会截断数据报并返回-1
        if (res == -1) {
            fprintf(stderr, "WSAError: %d\n", WSAGetLastError());
            continue;
        }
        buf[res] = '\0';
        fputs(buf, stdout);
    }
    puts("Client: recv over");
   
    closesocket(recvsock);
    WSACleanup();
    return 0;
}