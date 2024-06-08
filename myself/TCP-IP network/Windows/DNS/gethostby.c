#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h> //比winsock2更上层的头文件
/* 通过参数1获取对应的域名详细信息，可支持3种输入，分别是：
IPv4，由4组1字节的3位10进制数组成，使用点间隔，为0时可以省略
IPv6，由8组2字节的4位16进制数组成，使用冒号间隔，多组为0是可以简写为双冒号
域名，也就是网站的显示名称，不区分大小写，通过域名访问网络本质上需要通过DNS服务器来获取对应的IP地址进而访问 */

#define ErrorPuts(s) { \
    fputs(s, stderr); \
    fputc('\n', stderr); \
    exit(-2); \
}

#define BufSize 50

int main(int argc, char *args[])
{
    WSADATA wsadata;
    IN_ADDR addr_4;
    IN6_ADDR addr_6;
    PHOSTENT ph;
    int i;
    if (argc != 2)
    {
        printf("Usage: %s <addr>\n", args[0]);
        exit(-1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsadata)) //(winsock), 启动!
        ErrorPuts("WSAStartup() failed!");

    //分别处理3中输入形式以获取地址信息
    if ((addr_4.S_un.S_addr = inet_addr(args[1])) != INADDR_NONE) { //先假设输入时IPv4，获取其对应的地址信息，此返回值表明转换失败
        puts("-- IPv4 --");
        if (!(ph = gethostbyaddr((char *)&addr_4, sizeof(addr_4), AF_INET))) //用IP地址获取信息，可以指定v4和v6
            ErrorPuts("gethostbyaddr() error!");
    }
    else if (inet_pton(AF_INET6, args[1], addr_6.u.Byte) == 1) { //返回1表明格式是正确的IPv6，此函数也可用于v4
        puts("-- IPv6 --");
        if (!(ph = gethostbyaddr((char *)&addr_6, sizeof(addr_6), AF_INET6)))
            ErrorPuts("gethostbyaddr() error!");
    }
    else { //其他形式的输入均看做DNS
        puts("-- DNS --");
        if (!(ph = gethostbyname(args[1]))) { //用DNS获取信息
            ErrorPuts("gethostbyname() error!");
        }
    }

    //输出地址信息中的全部内容
    printf("Official DN: %s\n", ph->h_name); //官方域名
    for (i = 0; ph->h_aliases[i]; i++) //别名域名
        printf("Aliases DN %d: %s\n", i+1, ph->h_aliases[i]);
    if (ph->h_addrtype == AF_INET) { //IPv4
        printf("Address type: %s", "AF_INET\n"); //地址族
        for (i = 0; ph->h_addr_list[i]; i++) //IP地址，可能有多个IP共同承担一个网页的负载，注意此处的char**不是IP地址字符串数组，而是地址信息结构数组。由于有多个形式的地址结构，所以采用通用指针，那个时期没有void*
            printf("IP address %d: %s\n", i+1, inet_ntoa(*(PIN_ADDR)ph->h_addr_list[i]));
    }
    else if (ph->h_addrtype == AF_INET6) { //IPv6
        char buf[BufSize]; 
        printf("Address type: %s", "AF_INET6\n");
        for (i = 0; ph->h_addr_list[i]; i++) {
            inet_ntop(AF_INET6, (LPCVOID)ph->h_addr_list[i], buf, BufSize);
            printf("IP address %d: %s\n", i+1, buf);
        }
    }
    else { //其它。。。
        ErrorPuts("Usage: Unknow addr type!");
    }

    //获取本地计算机的标准主机名(DESKTOP-04AJ0EI)
    // char str[BufSize];
    // gethostname(str, BufSize);
    // printf("standard host name: %s\n", str);
}