#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
/* IOCP模型，win下对标epoll的IO模型，它依靠重叠IO组件、IOCP组件、多线程共同完成。基本步骤如下：
创建IOCP句柄，创建accept线程和客户端传输线程，而后通过accept线程接收客户端连接并将其绑定到IOCP上；
重叠IO函数完成后会添加到完成端口队列，等待客户端线程的IOCP获取函数处理，最后根据输入输出来做出不同行为 */

#define BUF_SIZE 1024
#define ErrorPuts(s) do { \
    printf("%s ErrorCode %d\n", s, WSAGetLastError()); \
    exit(-1); \
} while(0)

HANDLE hiocp; //IOCP的句柄，在main和2个子线程中都需要使用
int is_shutdown = FALSE; //服务端退出的标志，用来促使子线程退出

typedef struct io_info { //IO回调所需的信息，包含overlap结构，可在需要overlap传参的地方使用
    WSAOVERLAPPED overlap; //必须将overlap放在首字段，这样此结构即可作为overlap结构使用，又可作为io信息结构使用
    SOCKET sock; //回调使用的客户端套接字
    WSABUF wsabuf; //IO函数中参数所需的缓存区结构，其中的buf字段直接指向上述的buf字段作为缓存区内存，len字段为buf的大小
    enum { //标识IO完成时的状态，以便给出对应响应
        IOM_recv,
        IOM_send
    } iomode;
    int msgcount;
} IO_INFO;

//用于接收客户端连接请求的子线程，accept后还需recv，以此来触发后续客户子线程的运作
DWORD FT_Accept(LPVOID args);

//用于双端传输的子线程，
DWORD FT_Client(LPVOID args);

//释放客户端套接字和对应的资源，这些内容都在io_info中
void F_FreeSource(IO_INFO *info_p);

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET sersock, clisock;
    SOCKADDR_IN seraddr;
    LPWSAOVERLAPPED overlap_p; //overlap结构体，event字段可以注册事件进而完成事件驱动的重叠IO，也可以作为万能指针使用，其它字段留供服务提供商使用
    IO_INFO *info_p; //传给overlap中的event指针作为回调函数的所需IO信息
    SYSTEM_INFO sysinfo; //系统信息结构
    DWORD thd_count, recvsize, recvflag = 0; //1为客户端最大线程数；2为读取IO函数实际传递的字节数，可以不使用；3为设定IO函数的特性，必须使用并且需要初始化
    HANDLE *hthds; //线程数组，管理所以子线程
    LPVOID thdargs = &sersock; //传给accept线程的参数

    if (argc != 2) //端口号
    {
        printf("Usage: %s <Port>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() error!");
    GetSystemInfo(&sysinfo); //获取系统信息，这里只是用于获取cpu核数，以此来确定合理的线程数
    thd_count = sysinfo.dwNumberOfProcessors;
    hthds = (HANDLE *)malloc(sizeof(HANDLE)*thd_count+1);
    /* 创建IO完成端口，或句柄到绑定完成端口上。参数1为文件句柄，传入-1表示仅创建端口，传入一个已打开且支持重叠I/O的对象则表示绑定；
    参数2为完成端口，传空表创建，传已有端口表示绑定；参数3为附加信息，只用于将需要的信息传入后续的GetQueuedCompletionStatus函数中，可以将其作为指针值使用；
    参数4可指定最大线程数，0表示同处理器数目，它的含义是允许并发处理完成端口的最大线程数，任何线程都可以调用Get来处理完成端口，但能够同时运行(等待状态的线程不算)的线程不能超出此数
    成功返回完成端口句柄(新创建的或是参数2指定的)，失败返回NULL */
    if (!(hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thd_count)))
        ErrorPuts("CreateIoCompletionPort() error!");
    if ((sersock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        ErrorPuts("socket() error!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR)
        ErrorPuts("bind() error!");
    if (listen(sersock, 3) == SOCKET_ERROR)
        ErrorPuts("listen() error!");

    //分别创建accept线程和客户端线程，主线程只需管理这些线程即可，连接和传输的细节都交予线程
    hthds[0] = CreateThread(NULL, 0, FT_Accept, thdargs, 0, NULL);
    for (int i = 1; i <= thd_count; i++)
        hthds[i] = CreateThread(NULL, 0, FT_Client, NULL, 0, NULL);
    
    while (getchar() != 'q') //读取q字符作为服务端退出依据
        continue;
    is_shutdown = TRUE;
    for (int i = 1; i <= thd_count; i++) //给每个客户子线程发布iocp，使其从Get函数中退出
        PostQueuedCompletionStatus(hiocp, -1, 0, NULL); //Post函数会向iocp队列添加元素，进而被Get函数读取，参数2-4分别对应传入Get的参数2-4。这里传递参数2为-1来提示子线程退出
    if (WaitForMultipleObjects(thd_count, hthds+1, TRUE, INFINITE) == WAIT_FAILED) //先退出client线程
        ErrorPuts("WaitForMultipleObjects() error!");
    closesocket(sersock); //触发accept中断返回以结束子线程
    if (WaitForSingleObject(*hthds, INFINITE) == WAIT_FAILED) //再退出accept线程
        ErrorPuts("WaitForSingleObject() error!");
    WSACleanup();
    puts("Main: over!");
    return 0;
}

DWORD FT_Accept(LPVOID args)
{
    SOCKET sersock = *(SOCKET *)args, clisock;
    IO_INFO *info_p;
    DWORD recvflag = 0;
    while (!is_shutdown)
    {
        if ((clisock = accept(sersock, NULL, NULL)) == INVALID_SOCKET) {
            if (WSAGetLastError() == WSAEINTR) //此错误是本程序中断accept子线程的手段，当关闭sersock时会触发accept错误返回此码，进而退出
                break;
            else
                ErrorPuts("accept() error!");
        }
        printf("new client %llu\n", clisock);
        if (!CreateIoCompletionPort((HANDLE)clisock, hiocp, 0, 0)) //将客户端绑定到iocp上
            ErrorPuts("CreateIoCompletionPort() error!");
        info_p = (IO_INFO *)malloc(sizeof(IO_INFO));
        memset((void *)&info_p->overlap, 0, sizeof(info_p->overlap));
        info_p->sock = clisock;
        info_p->wsabuf.buf = (char *)malloc(BUF_SIZE);
        info_p->wsabuf.len = BUF_SIZE;
        info_p->iomode = IOM_recv;
        info_p->msgcount = 0;
        if ((WSARecv(clisock, &info_p->wsabuf, 1, NULL, &recvflag, &info_p->overlap, NULL) == SOCKET_ERROR)
            && (WSAGetLastError() != WSA_IO_PENDING))
                ErrorPuts("WSARecv() error!");
    }
    return 0;
}

DWORD FT_Client(LPVOID args)
{
    IO_INFO *info_p;
    ULONG_PTR compkey; //无用参数，但必须存在
    DWORD recvflag = 0, transsize;
    while (!is_shutdown)
    {
        /* 获取完成端口的状态，也就是阻塞等待iocp绑定的套接字的IO函数传输完成。完成IO的套接字会排入队列中等待此函数取出，当然，此函数所在线程的并发量受限于iocp创建函数的规定
        参数1为iocp句柄；参数2为具体的套接字IO传输完成的字节数；参数3返回完成信息，其值为CreateIoCompletionPort参数3的地址，可作为iocp传给线程的特殊信息来使用；
        参数4为IO函数传入的overlap结构地址指针，本程序不使用参数3，直接将所有io所需信息放入参数4所在结构中；参数5为超时时间。
        成功时返回非0，失败返回0。由于本函数是取出等待的IO完成队列，所以本函数的调用无需与IO函数的调用在时间尺度上对应，只需保证最终调用的次数一致即可 */
        if (!GetQueuedCompletionStatus(hiocp, &transsize, &compkey, (LPOVERLAPPED *)&info_p, INFINITE)) {
            if (WSAGetLastError() == ERROR_NETNAME_DELETED) { //此错误表示对端突然终止连接，这里删除客户端即可
                printf("client %llu error close!\n", info_p->sock);
                F_FreeSource(info_p);
                continue;
            }
            else
                ErrorPuts("GetQueuedCompletionStatus() error!");
        }
        if (transsize == -1) //子线程退出信号
            break;
        switch (info_p->iomode) //不同完成状态对应不同响应，输入完成就回声信息给客户端，输出完成则继续接收信息
        {
            case IOM_recv:
                if (transsize == 0) { //客户端关闭，清理相关内存
                    printf("close client %llu\n", info_p->sock);
                    F_FreeSource(info_p);
                }
                else { //客户端有信息传来，则echo信息
                    info_p->wsabuf.len = transsize; //精确设置回声信息的大小
                    info_p->iomode = IOM_send; //改变iomode
                    if (WSASend(info_p->sock, &info_p->wsabuf, 1, NULL, 0, &info_p->overlap, NULL) == SOCKET_ERROR)
                        if (WSAGetLastError() != WSA_IO_PENDING)
                            ErrorPuts("WSASend() error!");
                }
                break;
            case IOM_send:
                printf("echo %d message to client %llu with %lu byte\n", ++info_p->msgcount, info_p->sock, transsize);
                info_p->wsabuf.len = BUF_SIZE; //改回原大小
                info_p->iomode = IOM_recv;
                if (WSARecv(info_p->sock, &info_p->wsabuf, 1, NULL, &recvflag, &info_p->overlap, NULL) == SOCKET_ERROR)
                    if (WSAGetLastError() != WSA_IO_PENDING)
                        ErrorPuts("WSARecv() error!");
        }
    }
    return 0;
}

void F_FreeSource(IO_INFO *info_p)
{
    closesocket(info_p->sock);
    free(info_p->wsabuf.buf);
    free(info_p);
}