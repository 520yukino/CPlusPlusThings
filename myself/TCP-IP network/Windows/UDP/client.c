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
    // if (connect(clisock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR) //注意不同于TCP，此处的connect只是绑定服务器端地址信息，后续可以直接使用send而无需指定地址信息。当然，这不是必要操作
    //     errorputs("connect() failed");
    // puts("Client: send begin");
    // send(clisock, message, sizeof(message), 0);
    // puts("Client: send over");
    // if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //从服务器端接收数据
    //     errorputs("recv() failed!");
    // printf("Reply from server: %s\n", buf);

    //使用bind绑定客户端的情况
    // SOCKADDR_IN cliaddr;
    // memset(&cliaddr, 0, sizeof(cliaddr));
    // cliaddr.sin_family = AF_INET;
    // cliaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    // cliaddr.sin_port = htons(9999); //不使用8888端口，否则会bind失败，因为服务端绑定的就是8888
    // if (bind(clisock, (SOCKADDR *)(&cliaddr), sizeof(cliaddr)) == SOCKET_ERROR) //对于UDP而言，服务端和客户端的区分仅仅在于是否bind绑定了地址，绑定意味着一开始就可以阻塞接收，不绑定则必须先发送后接收，否则会出错
    //     errorputs("bind() failed!");
    // if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //对于客户端，未绑定的影响是无法一开始就进行接收，无论服务端是否有绑定，这种做法都是错误的。也就是必须满足先发送后接收，以及服务端处于绑定状态这2个条件，才能阻塞接收信息
    //     errorputs("recv() failed!");

    puts("Client: send begin");
    sendto(clisock, message, sizeof(message), 0, (SOCKADDR *)(&seraddr), sizeof(seraddr)); //需要指定客户端信息，注意程序如果先发送信息则会使该程序变为客户端，进而使得在服务端未绑定时recv失败，即使程序试图通过bind绑定为服务端也是如此
    puts("Client: send over");
    if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR) //可以直接recv而非recvfrom接收数据，只是这样做不知道发送者而已
        errorputs("recv() failed!");
    printf("Reply from server: %s\n", buf);

    closesocket(clisock);
    WSACleanup();
    return 0;
}