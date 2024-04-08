#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define errorputs(s) do { \
    puts(s); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET clisock; //客户端套接字
    SOCKADDR_IN seraddr; //套接字地址信息
    char message[] = "I'm client, Hello World!";
    const int SIZE = 1024;
    char *buf = (char *)malloc(SIZE);

    if (argc != 3) //需要传入IP和端口号
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        errorputs("WSAStartup() failed!");
    
    if ((clisock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) //创建客户端套接字
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = inet_addr(args[1]);
    seraddr.sin_port = htons(atoi(args[2]));

    //使用connect绑定客户端地址信息的情况
    // if (connect(clisock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR) //注意不同于TCP，此处的connect只是绑定服务器端地址信息，后续可以直接使用send和recv而无需指定地址信息，这不是必需的
    //     errorputs("connect() failed");
    // puts("Client: send begin 1");
    // send(clisock, message, sizeof(message), 0);
    // puts("Client: send over 1");
    // if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //从服务器端接收数据
    //     errorputs("recv() failed!");
    // printf("Reply from server 1: %s\n", buf);

    // 未connect的情况
    // SOCKADDR_IN cliaddr;
    // memset(&cliaddr, 0, sizeof(cliaddr));
    // cliaddr.sin_family = AF_INET;
    // cliaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    // cliaddr.sin_port = htons(9999);
    // if (bind(clisock, (SOCKADDR *)(&cliaddr), sizeof(cliaddr)) == SOCKET_ERROR) //即使先绑定地址，后续代码也必须先recv而非sendto，否则此进程依旧会变为客户端
    //     errorputs("bind() failed!");
    int len = sizeof(seraddr);
    puts("Client: send begin 2");
    sendto(clisock, message, sizeof(message), 0, (SOCKADDR *)(&seraddr), len); //需要指定客户端信息，注意程序如果先发送信息则会使该程序变为客户端，进而使得在服务端未绑定时recv失败，即使程序试图通过bind绑定为服务端也是如此
    puts("Client: send over 2");
    if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //可以直接recv而非recvfrom接收数据，只是这样做不知道发送者而已
        errorputs("recv() failed!");
    printf("Reply from server 2: %s\n", buf);

    closesocket(clisock);
    WSACleanup();
    return 0;
}