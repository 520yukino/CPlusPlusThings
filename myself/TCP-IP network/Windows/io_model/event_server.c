#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
//异步通知io模型，采用WSA组件中的EventSelect和事件驱动函数，由于需要io时发送的通知是基于事件的，所以程序无需单独阻塞等待io通知，而是直接等待事件通知，这样就可以处理所有的注册事件而非只是io
//异步是指添加事件和等待事件发生是分开的，指定监视对象后可以先做其它事情，而后再来检查事件状态。而像select则是添加和等待于一体，什么时候发生就什么时候返回

#define ErrorPuts(s) do { \
    printf("%s ErrorCode %lu\n", s, GetLastError()); \
    exit(-1); \
} while(0)

//关闭套接字连接和事件，维护套接字和事件的数组
void F_CloseConnect(SOCKET *sockarr, HANDLE *eventarr, int i, int sockcount);

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET sersock, clisock;
    SOCKADDR_IN seraddr, cliaddr;
    int addrsize = sizeof(cliaddr);
    char reply_1[] = "connecting";
    char reply_2[] = "over";
    const int SIZE = 1024;
    char *message = (char *)malloc(SIZE);
    int len;
    SOCKET sockarr[WSA_MAXIMUM_WAIT_EVENTS]; //套接字数组
    WSAEVENT eventarr[WSA_MAXIMUM_WAIT_EVENTS], event; //事件数组，后续需要使用的单个事件
    WSANETWORKEVENTS netevent; //WSAEnumNetworkEvents函数中储存事件类型的结构体
    int sockcount = 0; //套接字或事件的数量
    DWORD eventindex; //后续需要使用的事件数组下标

    if (argc != 2) //端口号
    {
        printf("Usage: %s <Port>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() failed!");
    
    if ((sersock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        ErrorPuts("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR)
        ErrorPuts("bind() failed!");
    if (listen(sersock, 3) == SOCKET_ERROR)
        ErrorPuts("listen() failed!");

    event = WSACreateEvent(); //创建事件，是CreateEvent的简化专用版，它直接创建manual-reset模式non状态的事件
    //添加事件，这个就是绑定套接字和事件的函数，参数3为监视的事件类型，此处与epoll的不同在于区分了套接字连接和关闭事件
    if (WSAEventSelect(sersock, event, FD_ACCEPT) == SOCKET_ERROR)
        ErrorPuts("WSAEventSelect() failed!");
    sockarr[sockcount] = sersock;
    eventarr[sockcount] = event;
    sockcount++;

    while (1) //主体循环
    {
        //wait阻塞等待多个事件，参数分别为：事件数目、事件数组、是否等待所有事件都发生、超时时间、是否为alertable-wait。失败返回WSA_WAIT_FAILED，当参数3为假，则成功时的返回值减去WSA_WAIT_EVENT_0就是参数2事件数组中发生了的事件的最小下标
        eventindex = WSAWaitForMultipleEvents(sockcount, eventarr, FALSE, WSA_INFINITE, FALSE)-WSA_WAIT_EVENT_0;
        for (int i = eventindex; i < sockcount; i++) //从最小下标开始到最后一个事件，逐一检查哪些事件发生了，而后做出处理
        {
            eventindex = WSAWaitForMultipleEvents(1, eventarr+i, TRUE, 0, FALSE); //非阻塞检查其中一个事件，这里可以看出，由于2次使用了wait函数，所以事件必须设置为manual模式
            if (eventindex == WSA_WAIT_TIMEOUT || eventindex == WSA_WAIT_FAILED) //non状态或失败则检查下一个
                continue;
            WSAEnumNetworkEvents(sockarr[i], eventarr[i], &netevent); //获取发生事件的事件类型，并将事件重置为non状态，事件类型储存到参数3中
            if (netevent.lNetworkEvents & FD_ACCEPT) { //必须用与运算判断是否有该事件，因为一个事件类型可以同时有多个事件发生，比如可读的同时客户端关闭，同理后续的事件类型判断也都要用if而非elseif
                if (netevent.iErrorCode[FD_ACCEPT_BIT]) { //iErrorCode[FD_XXX_BIT]如果非0，则表明FD_XXX事件发生错误
                    puts("event FD_ACCEPT error!");
                    continue;
                }
                if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &addrsize)) == -1) {
                    puts("accept() failed!");
                    continue;
                }
                //和前面一样创建并添加事件，但注意可读和关闭事件是分开的
                event = WSACreateEvent();
                if (WSAEventSelect(clisock, event, FD_READ|FD_CLOSE) == SOCKET_ERROR)
                    ErrorPuts("WSAEventSelect() failed!");
                sockarr[sockcount] = clisock;
                eventarr[sockcount] = event;
                sockcount++;
                send(clisock, reply_1, sizeof(reply_1), 0);
                printf("new client %llu\n", clisock);
                continue; //服务端只需要accept，后续是客户端的事件
            }
            if (netevent.lNetworkEvents & FD_READ) { //可读，这里没有采用循环读入至清空缓冲区，所以可能发生缓冲区尚未读空就关闭的现象
                if (netevent.iErrorCode[FD_READ_BIT]) {
                    puts("event FD_READ error!");
                    continue;
                }
                if ((len = recv(sockarr[i], message, SIZE, 0)) == -1) {
                    printf("client %llu recv() failed!\n", sockarr[i]);
                    F_CloseConnect(sockarr, eventarr, i, sockcount--);
                    continue;
                }
                else {
                    printf("message from client %llu of %d byte: %s\n", sockarr[i], len, message);
                }
            }
            if (netevent.lNetworkEvents & FD_CLOSE) { //关闭
                if (netevent.iErrorCode[FD_CLOSE_BIT]) { //这里的错误包括客户端异常关闭
                    puts("event FD_CLOSE error!");
                    continue;
                }
                send(sockarr[i], reply_2, sizeof(reply_2), 0);
                printf("close client %llu\n", sockarr[i]);
                F_CloseConnect(sockarr, eventarr, i, --sockcount); //要提前自减sockcount，因为函数内部会将其当做尾部元素下标来使用
            }
        }
    }

    closesocket(sersock);
    WSACleanup();
    return 0;
}

void F_CloseConnect(SOCKET *sockarr, HANDLE *eventarr, int i, int sockcount)
{
    WSACloseEvent(eventarr[i]);
    closesocket(sockarr[i]);
    sockarr[i] = sockarr[sockcount];
    eventarr[i] = eventarr[sockcount];
}