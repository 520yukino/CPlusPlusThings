#include <stdio.h>
#pragma pack(1) //设定内存对齐的字节数

struct S1
{
    char a;
    int b;
    int c;
    char d;
    char e;
    char f;
    int g;
};

struct S2
{
    int a:1;
    int b:2;
    int c:7;
};

void Func1()
{
    int a, b;
    printf("Func1(): %p, %p\n", &a, &b);
}

int main()
{
    struct S1 s1;
    printf("align s1: %lu, %p, %p, %p, %p, %p, %p, %p\n", sizeof(struct S1), &s1.a, &s1.b, &s1.c, &s1.d, &s1.e, &s1.f, &s1.g);
    Func1();
    struct S2 s2;
    int a1;
    printf("align s2: %lu, %p, %p, %p\n", sizeof(struct S2), &s1, &s2, &a1); //不允许采用位域的地址
}
