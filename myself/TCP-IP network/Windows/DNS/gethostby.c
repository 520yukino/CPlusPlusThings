#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <in6addr.h>

#define ErrorPuts(s) { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
}

int main(int argc, char *args[])
{
    WSADATA wsadata;
    int i;
    PHOSTENT ph;
    if (argc != 2)
    {
        printf("Usage: %s <addr>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata)) //(winsock), 启动!
        ErrorPuts("WSAStartup() failed!");

    if (args[1][0] < '0' || args[1][0] > '9') { //通过域名来获取其对应的域名信息
        if ((ph = gethostbyname(args[1])) == NULL)
            ErrorPuts("gethostbyname error!");
    }
    else { //通过IP地址来获取
        IN_ADDR addr;
        memset(&addr, 0, sizeof(addr));
        addr.S_un.S_addr = inet_addr(args[1]);
        if ((ph = gethostbyaddr((char *)&addr, 4, AF_INET)) == NULL) //暂不支持IPv6
            ErrorPuts("gethostbyaddr error!");
    }
    
    printf("Official DN: %s\n", ph->h_name); //官方域名
    for (i = 0; ph->h_aliases[i]; i++) //别名域名
        printf("Aliases DN %d: %s\n", i+1, ph->h_aliases[i]);
    if (ph->h_addrtype == AF_INET) { //IPv4
        printf("Address type: %s", "AF_INET\n"); //地址族
        for (i = 0; ph->h_addr_list[i]; i++) //IP地址，可能有多个IP共同承担一个网页的负载，注意此处的char*不是IP地址，而是地址结构指针，那个时期没有void*
            printf("IP address %d: %s\n", i+1, inet_ntoa(*(PIN_ADDR)ph->h_addr_list[i]));
    }
    else if (ph->h_addrtype == AF_INET6) { //IPv6
        printf("Address type: %s", "AF_INET6\n");
        for (i = 0; ph->h_addr_list[i]; i++)
            printf("IP address %d: %s\n", i+1, ((PIN6_ADDR)(ph->h_addr_list[i]))->u.Byte); //暂时无法使用IPv6中转换为字符串的函数inet_ptoa
    }
    else { //其它。。。
        ErrorPuts("Usage: Unknow addr type!");
    }
}