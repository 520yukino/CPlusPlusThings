#include <stdio.h>
#include <stdlib.h>
union U
{
    int a;
    short b;
    int c:16;
};

int a = 1234; // 全局区
int main()
{
    const char *b = "Hello";                   // 指针指向代码区，b变量本身也在栈区
    int c = 5678;                        // 栈区
    int *d = (int *)malloc(sizeof(int)); // 指针指向堆区，d变量本身同样在栈区
    *d = 9999;
    static char e = 0;        // 全局区
    static int (*f)() = main; // 指向代码区，f本身在全局区

    printf("&a = %p, a = %d\n", &a, a);
    printf(" b = %p, b = %s, &b = %p\n", b, b, &b);
    printf("&c = %p, c = %d\n", &c, c);
    printf(" d = %p, *d = %d, &d = %p\n", &d, *d, &d);

    printf("&e = %p, e = %d\n", &e, e);
    printf("&f = %p, f = %p\n", &f, f);
    printf("&main = %p, main = %p\n", &main, main);

    U u = {0x1000f}; //本机为小端序，所以b为a的低2位
    printf("u.b = %d\n", u.b);
    return 0;
}
