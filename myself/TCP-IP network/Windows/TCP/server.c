#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") //只对Visual Studio有效，这里只能使用命令行-lws2_32的方式链接此库

#define ErrorPuts(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET sersock, clisock; //套接字句柄，TCP服务器端可以同时连接多个客户端
    SOCKADDR_IN seraddr, cliaddr; //套接字地址信息
    int szcliaddr = sizeof(cliaddr);
    char reply_1[] = "connecting";
    char reply_2[] = "over";
    const int SIZE = 1024;
    char *message = (char *)malloc(SIZE);

    if (argc != 2) //需要传入端口号
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata)) //(winsock), 启动!
        ErrorPuts("WSAStartup() failed!");
    
    if ((sersock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) //创建服务器端套接字，参数分别为协议(地址)族、套接字类型、协议类型，此处相当于使用TCP协议
        ErrorPuts("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr)); //必须设置seraddr的其它无用位为0，sockaddr结构是通用套接字地址结构，而sockaddr_in是ipv4独有的，其中的sin_zero字段是占位段
    seraddr.sin_family = AF_INET; //地址族，本质上和PF_INET没有区别
    seraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //将本机(host)cpu解析数据的方式小端序转为网络(network)传输统一使用的大端序，INADDR_ANY为自动分配IP，若只有一个网卡则可以这么做
    seraddr.sin_port = htons(atoi(args[1])); //l为long，表IP；s为short，表端口，端口号实则是为了区分服务器端的套接字
    if (bind(sersock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR) //绑定套接字地址信息，注意需要转换地址类型为通用类型
        ErrorPuts("bind() failed!");
    if (listen(sersock, 3) == SOCKET_ERROR) //转为可连接状态，之后connect可加入等待队列，参数而为队列最大数，即最多能有n个客户端等待accept，1个正在连接，n+1个最大客户端数
        ErrorPuts("listen() failed!");

    const int SERVER_COUNT = 5; //最大服务器连接次数
    int mc, res; //信息数，recv的返回值
    for (int i = 1; i <= SERVER_COUNT; i++) //循环服务
    {
        mc = 0;
        if ((clisock = accept(sersock, (SOCKADDR *)(&cliaddr), &szcliaddr)) == INVALID_SOCKET) //取出队列中的客户，如果没有就阻塞接受客户端的连接请求
            ErrorPuts("accept() failed!");
        printf("Server: connecting client %d\n", i);
        // Sleep(1000); //此处的sleep会让client阻塞在recv
        puts("Server: reply_1");
        send(clisock, reply_1, sizeof(reply_1), 0); //通知客户端已联通，注意发送数据不会等待对方的接收，而是直接发送至自身的输出缓冲区后就返回，之后输出缓冲区会发送至对方的输入缓冲区中等待对方recv读入
        while ((res = recv(clisock, message, SIZE, 0)) != 0) //从客户端接收数据，接收数据会等待至数据发送过来为止。注意recv会直接一次性读入规定的大小而不管是否含有\0和EOF，正确读入时返回值为读入的字符数，当另一方关闭时会发送EOF，此时该函数返回0
        {
            if (res == SOCKET_ERROR)
            {
                ErrorPuts("recv() failed!");
                break;
            }
            if (!strcmp(message, "exit") || !strcmp(message, "quit"))
                break;
            printf("Message %d from client %d of %d byte(s): %s\n", ++mc, i, res, message);
        }
        puts("Server: reply_2");
        send(clisock, reply_2, sizeof(reply_2), 0); //通知客户端接收完毕，注意发送0字节是无意义的，对方recv不会响应
        // shutdown(clisock, SD_SEND); //shutdown可以关闭指定的输入输出流，这里关闭了服务器端的输出流(也是客户端的输入流)，注意第一个参数仍然是客户端套接字
        // 直接close断开连接可能不保险，如果对方还有数据需要传入到服务器，close后是无法接收的，可以使用上述的shutdown只断开输出流保留输入流，注意关闭输出流的操作会向对方发送EOF，而关闭输入流则不会
        closesocket(clisock); //关闭套接字会关闭输入输出流，但会保留输出缓冲区数据以继续向对方传输，关闭输入缓冲区
        printf("Server: closed client %d\n", i);
    }

    closesocket(sersock); //主动断开的一方正常情况下最终会处于TIME_WAIT状态等待2MSL再CLOSE。对于服务端，同一端口的使用不会受TIME_WAIT的影响，但对于客户端，TIME_WAIT会导致端口占用进而connect失败
    WSACleanup(); //注意如果使用中断来结束进程，则WSA组件也会被清除，这会导致客户端的recv返回错误值，而非服务端正常close时客户端接收EOF进而返回0，这与Linux有差异，因为Linux没有WSA，中断时可以正常close
    return 0;
}