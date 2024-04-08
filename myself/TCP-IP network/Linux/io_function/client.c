#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    int clisock;
    struct sockaddr_in seraddr;
    const int SIZE = 1024;
    char *message = (char *)malloc(SIZE);
    message = "I'm client, Hello World!";
    char *buf = (char *)malloc(SIZE);

    if (argc != 3)
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    
    if ((clisock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr(args[1]);
    seraddr.sin_port = htons(atoi(args[2]));

    // int optval, optlen = sizeof(optval);
    // getsockopt(clisock, SOL_SOCKET, SO_OOBINLINE, (char *)&optval, &optlen);
    // printf("SO_OOBINLINE = %d\n", optval);
    // optval = 1;
    // setsockopt(clisock, SOL_SOCKET, SO_OOBINLINE, (char *)&optval, optlen);

    if (connect(clisock, (struct sockaddr *)(&seraddr), sizeof(seraddr)) == -1)
        errorputs("connect() failed");
    if (read(clisock, buf, SIZE) == -1)
        errorputs("recv() failed!");
    printf("Reply from server: %s\n", buf);
    puts("send 1");
    write(clisock, message, SIZE);
    send(clisock, "urgmsg 1", SIZE, MSG_OOB); //发送紧急信息，out-of-band带外传输数据
    write(clisock, message, SIZE);
    send(clisock, "urgmsg 2", SIZE, MSG_OOB);
    write(clisock, message, SIZE);
    puts("send 2");
    sleep(1);

    shutdown(clisock, SHUT_WR);
    if (read(clisock, buf, SIZE) == -1)
        errorputs("recv() failed!");
    printf("Reply from server: %s\n", buf);
    return 0;
}