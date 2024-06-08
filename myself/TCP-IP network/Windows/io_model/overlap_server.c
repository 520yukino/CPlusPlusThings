#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
//重叠IO模型，采用WSA组件专门提供的套接字和IO函数。IO函数是非阻塞的，后续结果获取可通过overlap结构注册的事件来通知，或者通过CompRout机制调用回调函数来处理
//此模型是win独有的，它既可以完成readv和writev这样的多缓存IO，也可以达到类似异步通知IO模型的效果
//本程序为回声服务器，收到客户端输入后马上回复相同内容，而后继续接收，每个客户端都有独立的IO信息结构和回调函数

#define BUF_SIZE 1024
#define ErrorPuts(s) do { \
    printf("%s ErrorCode %lu\n", s, WSAGetLastError()); \
    exit(-1); \
} while(0)

typedef struct io_info { //IO回调所需的信息，可通过overlap结构中的event句柄(void *)来传递给回调函数
    SOCKET sock; //回调使用的客户端套接字
    WSABUF wsabuf; //IO函数中参数所需的缓存区结构，其中的buf字段直接指向上述的buf字段作为缓存区内存，len字段为buf的大小
} IO_INFO;

//WSARecv函数的完成路径参数设置的函数，接收信息完成后该函数回响发送信息至客户端
void F_RecvCompRout(DWORD errcode, DWORD transsize, LPWSAOVERLAPPED overlap_p, DWORD flags);

//WSASend函数的完成路径参数设置的函数，回响发送信息完成后该函数继续接收客户端信息
void F_SendCompRout(DWORD errcode, DWORD transsize, LPWSAOVERLAPPED overlap_p, DWORD flags);

//释放客户端套接字和对应的资源，io_info在overlap的event字段中
void F_FreeSource(LPWSAOVERLAPPED overlap_p);

int main(int argc, char *args[])
{
    WSADATA wsadata;
    SOCKET sersock, clisock;
    SOCKADDR_IN seraddr;
    LPWSAOVERLAPPED overlap_p; //overlap结构体，event字段可以注册事件进而完成事件驱动的重叠IO，也可以作为万能指针使用，其它字段留供服务提供商使用
    IO_INFO *info_p; //传给overlap中的event指针作为回调函数的所需IO信息
    u_long ctlmode = 1; //设置ioctl的参数3
    DWORD recvsize, recvflag = 0; //1为读取IO函数实际传递的字节数，可以不使用；2为设定IO函数的特性，必须使用并且需要初始化

    if (argc != 2) //端口号
    {
        printf("Usage: %s <Port>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
        ErrorPuts("WSAStartup() error!");
    //此套接字创建函数扩展了重叠IO的功能，参数1-3不变，4和5无需理会，参数6设置属性信息，此处设置重叠IO
    if ((sersock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        ErrorPuts("socket() error!");
    ioctlsocket(sersock, FIONBIO, &ctlmode); //类似于linux下的fcntl，控制套接字的IO模式，此处设置服务端为非阻塞。
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (SOCKADDR *)(&seraddr), sizeof(seraddr)) == SOCKET_ERROR)
        ErrorPuts("bind() error!");
    if (listen(sersock, 3) == SOCKET_ERROR)
        ErrorPuts("listen() error!");

    while (1) //主体循环
    {
        SleepEx(10, TRUE); //使用可以触发alertale_wait的阻塞函数，阻塞过程中可以被CompRout回调函数打断。除此函数外还有WaitForXXXObjectsEx系列的2个函数和WSAWaitForMultipleEvents具有此功能。这些函数在调用完回调函数后都返回WAIT_IO_COMPLETION表示成功调用
        if ((clisock = accept(sersock, NULL, NULL)) == INVALID_SOCKET) { //非阻塞模式下若未连接成功则返回错误，后续可判断是由于没有客户端连入还是函数出错，进而处理
            if (WSAGetLastError() == WSAEWOULDBLOCK) //accept正常退出，没有客户端连入，则返回sleep继续阻塞等待回调打断和再次测试客户端连入
                continue;
            else
                ErrorPuts("accept() error!");
        }
        printf("new client %llu\n", clisock);
        //有客户端连入，则创建所需信息并接收信息
        overlap_p = (LPWSAOVERLAPPED)calloc(1, sizeof(WSAOVERLAPPED)); //对于此结构体，必须初始化事件字段为可访问指针或空，否则后续访问该事件指针时可能由于非法地址而错误
        info_p = (IO_INFO *)malloc(sizeof(IO_INFO));
        info_p->sock = clisock;
        info_p->wsabuf.buf = (char *)malloc(BUF_SIZE);
        info_p->wsabuf.len = BUF_SIZE;
        overlap_p->hEvent = (HANDLE)info_p; //本例没有使用事件驱动，所以event可以作为所需信息的指针来传入回调函数
        /* WSA推出的recv和send函数添加了更多功能，参数2为WSABUF指针；参数3为BUF的数量，由这2个参数即可达成类似readv和writev函数的功能，来分组读写信息；
        参数4为读写内容的字节数，此函数是非阻塞的，读写会异步完成，所以此参数在未完成读写时不会返回读写的字节数；参数5是设置IO特性，如OOB，recv中还会返回接收到的信息设置的特性；
        参数6是overlap结构指针，如要使用重叠IO则必须传入；参数7指定传输完成后且程序处于alertable_wait状态时调用的回调函数，此函数的4个参数如下：参数1为传输错误码，参数2为IO传输的字节数，参数3为IO函数传入的overlap结构，参数4为IO函数设置的特性；
        返回值和普通IO函数基本无异，但如果重叠IO尚未完成传输，则依旧会返回错误，WSAGetLastError会返回WSA_IO_PENDING。
        回调内部可以再次递归调用IO函数，因为回调是在IO完成后才调用的，本程序就是使用send和recv的反复回调完成循环回声IO的
        此函数是重叠IO的关键，本例采用CompRout回调来实现，如果使用事件驱动，则可以使用WSAGetOverlappedResult函数来获取传输结果 */
        if (WSARecv(clisock, &info_p->wsabuf, 1, &recvsize, &recvflag, overlap_p, F_RecvCompRout) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) //此错误代码表示数据暂未传输完成，不是错误，其它情况才是错误
                ErrorPuts("WSARecv() error!");
        }
    }

    closesocket(sersock);
    WSACleanup();
    return 0;
}

void F_RecvCompRout(DWORD errcode, DWORD transsize, LPWSAOVERLAPPED overlap_p, DWORD flags)
{
    printf("F_Recv: errcode = %d, transsize = %d\n", errcode, transsize);
    IO_INFO *info_p = (IO_INFO *)overlap_p->hEvent;
    if (errcode) { //传输有错误
        if (errcode == WSAECONNRESET) { //如果原因是客户端强行终止连接，则关闭客户端，并且直接退出函数
            printf("client %llu error close!\n", info_p->sock);
            F_FreeSource(overlap_p);
            return;
        }
        else //其它错误
            ErrorPuts("F_RecvCompRout() error!");
    }
    if (!transsize) { //客户端关闭，清理相关内存
        printf("close client %llu\n", info_p->sock);
        F_FreeSource(overlap_p);
    }
    else { //客户端有信息传来，则echo信息
        info_p->wsabuf.len = transsize; //可以为了节省数据传输资源而精确设置回声信息的大小，但后续recv之前必须改回来
        //send函数和recv的参数及返回值基本一致，overlap直接传入，回调函数对应即可
        if (WSASend(info_p->sock, &info_p->wsabuf, 1, NULL, 0, overlap_p, F_SendCompRout) == SOCKET_ERROR)
            if (WSAGetLastError() != WSA_IO_PENDING)
                ErrorPuts("WSASend() error!");
    }
}

void F_SendCompRout(DWORD errcode, DWORD transsize, LPWSAOVERLAPPED overlap_p, DWORD flags)
{
    static int n = 0;
    printf("F_Send: errcode = %d, transsize = %d, n = %d\n", errcode, transsize, ++n);
    IO_INFO *info_p = (IO_INFO *)overlap_p->hEvent;
    DWORD recvflag = 0;
    if (errcode) { //同recv回调
        if (errcode == WSAECONNRESET) {
            printf("client %llu error close!\n", info_p->sock);
            F_FreeSource(overlap_p);
            return;
        }
        else
            ErrorPuts("F_SendCompRout() error!");
    }
    info_p->wsabuf.len = BUF_SIZE; //改回原大小
    if (WSARecv(info_p->sock, &info_p->wsabuf, 1, NULL, &recvflag, overlap_p, F_RecvCompRout) == SOCKET_ERROR)
        if (WSAGetLastError() != WSA_IO_PENDING)
            ErrorPuts("WSARecv() error!");
}

void F_FreeSource(LPWSAOVERLAPPED overlap_p)
{
    IO_INFO *info_p = (IO_INFO *)overlap_p->hEvent;
    closesocket(info_p->sock);
    free(info_p->wsabuf.buf);
    free(info_p);
    free(overlap_p);
}