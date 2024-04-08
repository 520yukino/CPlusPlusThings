#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
//基于epoll的IO复用服务端，epoll专用于Linux，是select的强化版，它将fd集合交给内核管理而非用户，这样无需反复传入fd_set，而是调用epoll_ctl更改fd集合，适用于高并发服务器
//select和epoll都默认使用条件触发而非边缘触发，前者只要缓冲区中还有数据就会不断发生事件，后者只在刚接收到数据时发生事件

#define errorputs(s) do { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
} while(0)

int main(int argc, char* args[])
{
    int sersock, clisock;
    struct sockaddr_in seraddr, cliaddr;
    socklen_t szcliaddr = sizeof(cliaddr);
    char reply_1[] = "connecting";
    char reply_2[] = "over";
    const int SIZE = 1024;
    char* message = (char*)malloc(SIZE);
    int len;
    struct epoll_event *event_group, event; //epoll发生事件集合体，临时事件结构体
    int event_num, epfd, fdtemp, timeout = 5000; //发生的事件数，epoll专用fd，临时fd，epoll_wait等待的超时毫秒数
    const int EPOLL_SIZE = 100; //epoll事件集合体的大小

    if (argc != 2)
    {
        printf("Usage: %s: invalid argc!\n", args[0]);
        exit(-1);
    }
    if ((sersock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        errorputs("socket() failed!");
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(atoi(args[1]));
    if (bind(sersock, (struct sockaddr*)(&seraddr), sizeof(seraddr)) == -1)
        errorputs("bind() failed!");
    if (listen(sersock, 5) == -1)
        errorputs("listen() failed!");

    epfd = epoll_create(EPOLL_SIZE); //创建epoll，返回epoll专用fd，参数指定epoll大小，现已弃用
    event_group = (struct epoll_event *)malloc(EPOLL_SIZE*sizeof(struct epoll_event)); //事件集合体最好用动态内存
    event.events = EPOLLIN; //设置服务器的输入事件，一旦有连接请求就会反应
    event.data.fd = sersock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sersock, &event); //添加该事件

    while (1) //服务端与客户端循环连接的主体
    {
        if ((event_num = epoll_wait(epfd, event_group, EPOLL_SIZE, timeout)) == -1) //监视服务端的输入，因为客户端的连接请求本就需要发送信息，所以将select放在accept前面
            errorputs("epoll_wait() failed!");
        else if (event_num == 0) {
            puts("timeout...");
            continue;
        }
        puts("epoll_wait return");
        for (int i = 0; i<event_num; i++) //查询所有发生的事件
        {
            fdtemp = event_group[i].data.fd;
            if (fdtemp == sersock) { //服务器fd可读，说明有客户端请求连接
                if ((clisock = accept(sersock, (struct sockaddr*)(&cliaddr), &szcliaddr)) == -1) {
                    puts("accept() failed!");
                    continue;
                }
                printf("new client %d\n", clisock);
                write(clisock, reply_1, sizeof(reply_1));
                fcntl(clisock, F_SETFL, fcntl(clisock, F_GETFL, 0)|O_NONBLOCK); //设置非阻塞客户端，先获取文件属性而后再与上O_NONBLOCK
                event.events = EPOLLIN; //设置客户端的输入事件，边缘触发使得客户端有数据发送来才会反应
                event.data.fd = clisock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clisock, &event); //添加该事件
            }
            else { //客户端fd可读，意味着客户端发送了信息
                while(1) { //循环读空输入缓冲区
                    len = read(fdtemp, message, SIZE);
                    if (len == -1) { //输入出错
                        if (errno == EAGAIN) //这个并非真正的错误，非阻塞情况下如果read为空则返回-1并置errno为EAGAIN，这说明已经读空输入缓冲区
                            break;
                        printf("client %d read() failed!\n", fdtemp);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fdtemp, NULL); //删除该事件，此时无需事件信息
                        close(fdtemp);
                        break;
                    }
                    else if (len == 0) { //客户端关闭
                        write(fdtemp, reply_2, sizeof(reply_2));
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fdtemp, NULL); //删除该事件
                        close(fdtemp);
                        printf("close client %d\n", fdtemp);
                        break;
                    }
                    else {
                        printf("message from client %d of %d byte: %s\n", fdtemp, len, message);
                    }
                }
            }
        }
    }

    close(epfd);
    close(sersock);
    return 0;
}