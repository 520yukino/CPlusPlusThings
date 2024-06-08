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
    char *buf = (char *)malloc(SIZE);
    char *message[] = {"I'm client, Hello World!", "urgmsg 1", "urgmsg 2"};
    int msglen[] = {strlen(message[0]), strlen(message[1]), strlen(message[2])};

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
    puts("send begin");
    write(clisock, message[0], msglen[0]);
    usleep(1000);
    send(clisock, message[1], msglen[1], MSG_OOB); //发送紧急信息，out-of-band带外传输数据
    usleep(1000);
    write(clisock, message[0], msglen[0]);
    usleep(1000);
    send(clisock, message[2], msglen[2], MSG_OOB);
    usleep(1000);
    write(clisock, message[0], msglen[0]);
    puts("send over");

    shutdown(clisock, SHUT_WR);
    if (read(clisock, buf, SIZE) == -1)
        errorputs("recv() failed!");
    printf("Reply from server: %s\n", buf);
    return 0;
}