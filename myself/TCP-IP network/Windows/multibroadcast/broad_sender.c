#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h> //用于IP_MULTICAST_TTL
//广播，利用UDP发送数据给所有在该网络ID范围内的主机，直接广播是将主机ID所有位置1，本地广播将IP地址所有位置1
#define ErrorPuts(s) do { \
    printf("%s - errorcode: %d", WSAGetLastError()); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET broadsock;
    SOCKADDR_IN broadaddr;
    int addrsize = sizeof(broadaddr);
    const int SIZE = 1024;
    char *buf = (char *)malloc(SIZE);
    int so_bro = 1;
    FILE *fp;

    if (argc != 3) //需要传入广播IP和端口号
    {
        printf("Usage: %s: <GroupIP> <Port>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup()");
    
    if ((broadsock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) //广播采用UDP传输方式
        ErrorPuts("socket()");
    memset(&broadaddr, 0, sizeof(broadaddr)); //设置广播地址
    broadaddr.sin_family = AF_INET;
    broadaddr.sin_addr.S_un.S_addr = inet_addr(args[1]);
    broadaddr.sin_port = htons(atoi(args[2]));
    //设置广播模式开启(这里是默认开启的)，广播与普通UDP唯一的不同点就是这里，注意广播如果不置主机ID的所有位为1则就相当于普通的UDP，所以使用本机IP(UDP)、本机网络IP.255(直接广播)、255.255.255.255(本地广播)这3种IP都可以发送给本机
    if (setsockopt(broadsock, SOL_SOCKET, SO_BROADCAST, (char *)(&so_bro), sizeof(so_bro)) == SOCKET_ERROR)
        ErrorPuts("setsockopt()");
    if (connect(broadsock, (SOCKADDR *)(&broadaddr), addrsize) == SOCKET_ERROR) //持续使用广播地址
        ErrorPuts("connect()");
    if ((fp = fopen("buf.txt", "r")) == NULL)
        ErrorPuts("fopen()");

    puts("Sender: send begin");
    while (!feof(fp))
    {
        fgets(buf, SIZE, fp);
        send(broadsock, buf, SIZE, 0);
        Sleep(200);
    }
    puts("Sender: send over");

    closesocket(broadsock);
    WSACleanup();
    return 0;
}