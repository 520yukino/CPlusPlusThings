#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h> //用于IP_MULTICAST_TTL
//多播，利用UDP发送数据到加入了多播组的所有主机，sender类似于客户端而非服务端，因为它只负责发送信息给多播地址，而receiver需要提前运行并等待接收信息，这类似服务端
#define ErrorPuts(s) do { \
    puts(s); \
    exit(-2); \
} while(0)

int main(int argc, char *args[]) 
{
    WSADATA wsadata;
    SOCKET multisock;
    SOCKADDR_IN multiaddr;
    int addrsize = sizeof(multiaddr);
    const int SIZE = 1024;
    char *buf = (char *)malloc(SIZE);
    int ttl = 64; //最大生存时间，即数据包在路由器间的传递最大次数
    FILE *fp;

    if (argc != 3) //需要传入多播IP和端口号
    {
        printf("Usage: %s: <GroupIP> <Port>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    
    if ((multisock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) //多播采用UDP传输方式
        ErrorPuts("socket() failed!");
    memset(&multiaddr, 0, sizeof(multiaddr)); //设置多播地址
    multiaddr.sin_family = AF_INET;
    multiaddr.sin_addr.S_un.S_addr = inet_addr(args[1]);
    multiaddr.sin_port = htons(atoi(args[2]));
    if (connect(multisock, (SOCKADDR *)(&multiaddr), addrsize) == SOCKET_ERROR) //持续使用多播地址
        ErrorPuts("connect() failed!");
    setsockopt(multisock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)(&ttl), sizeof(ttl)); //设置多播模式，多播属于IP层，需要传入TTL以设置生存时间
    if ((fp = fopen("buf.txt", "r")) == NULL)
        ErrorPuts("fopen() failed!");

    puts("Sender: send begin");
    while (!feof(fp))
    {
        fgets(buf, SIZE, fp);
        send(multisock, buf, SIZE, 0);
        Sleep(200);
    }
    puts("Sender: send over");

    closesocket(multisock);
    WSACleanup();
    return 0;
}