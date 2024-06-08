#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>

#define ErrorPuts(s) do { \
    puts(s); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET clisock;
    SOCKADDR_IN seraddr;
    char message[5000] = "I'm client, Hello World!";
    const int SIZE = 1024;
    char *buf = (char *)malloc(SIZE);

    if (argc != 3)
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(3, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    
    if ((clisock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        ErrorPuts("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = inet_addr(args[1]);
    seraddr.sin_port = htons(atoi(args[2]));

    if (connect(clisock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR)
        ErrorPuts("connect() failed");
    if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR)
        ErrorPuts("recv() failed!");
    printf("Reply from server: %s\n", buf);
    puts("Client: send begin");
    srand(time(NULL));
    for (int i = 0; i < (rand()%5+1); i++)
    {
        send(clisock, message, sizeof(message), 0);
        Sleep(200);
    }
    puts("Client: send over");

    shutdown(clisock, SD_SEND); //关闭输出流保留输入流，接收服务端最后的数据
    if (recv(clisock, buf, SIZE, 0) == SOCKET_ERROR)
        ErrorPuts("recv() failed!");
    printf("Reply from server: %s\n", buf);
    WSACleanup();
    return 0;
}