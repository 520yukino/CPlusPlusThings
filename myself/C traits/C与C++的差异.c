#include <stdio.h>
struct S
{
    int* a;
};
void f() {} //普通函数必须把函数声明放在调用之前，但C++中的类却例外，类方法可以在另一个类方法声明之前调用它
int main()
{
    int a1 = 22;
    struct S s1 = {&a1}; //C中结构体不能直接像C++对象那样使用默认构造函数赋值
    a1 = *s1.a;
    f();
    int a2[0];
    printf("a1[0] = %d\n", a2);
    char * c1 = NULL; //C中的NULL被替换为((void *)0)，但C++中却是0，原因是函数重载会出现问题（void*不知道该转换为什么类型的参数合适）
}

