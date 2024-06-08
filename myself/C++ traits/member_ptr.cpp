#include <iostream>
#include <functional>
#define TEST_1
#ifdef TEST_1
//成员指针并非属于某一对象，而是整个类，所以它的功能不太寻常，就好似类的专用函数指针
//函数指针无法指向非静态成员函数，因为这类函数中有隐含的this指针，而成员指针恰恰相反

struct C1
{
    void t(int n)
    {
        std::cout << "in t(int): ";
        for (; n; n--)
            putchar('t');
        putchar('\n');
    }
    void t()
    {
        std::cout << "in t()" << std::endl;
    }
    void f2() {}
    C1 *operator->() //重载->，注意必须返回类指针以供默认->运行
    {
        std::cout << "in operator->()" << std::endl;
        return this;
    }
    static void f1() {std::cout << "in static f1()" << std::endl;}
    static int b;
    int c1, c2, c3;

private:
    int a = 1;
};

int C1::b = 114514;

class C2: public C1 {};
class C3: virtual public C1 {};

void ff() {}

int main()
{
    C1 c1, c2;
    auto bf1 = std::bind(static_cast<void (C1::*)(int)>(&C1::t), c1, 3); //强转成员指针来绑定重载成员函数
    auto bf2 = std::bind(&C1::f2, c1); //普通成员函数绑定
    auto bf3 = std::bind(C1::f1); //静态成员函数绑定
    bf1(); //调用t(3)版本
    bf2();
    bf3();
    c1->t(); //此为调用重载的->，其等效与(c1.operator->())->t();，注意是对象本身调用运算符而非指针调用，它可以在调用t之前先执行重载->中的内容

    void (C1::*p1)() = &C1::t; //有重载时不能auto，需要确定准确的函数类型
    (c1.*p1)(); //和函数指针一样，不能使用重载功能
    auto p2 = &C1::f1; //此为函数指针而非成员指针，如果f1不是static则p2为成员指针，因为静态函数是整个类所有而非某一对象
    auto p3 = &C1::operator->;
    auto p4 = &C1::b; //数据成员同样如此，静态成员为普通指针，非静态为成员指针
    int C1::*p5[3] = { &C1::c1, &C1::c2, &C1::c3 };
    auto p6 = ff;
    printf("f1: %p, %p | b: %p, %p\n", C1::f1, p2, &C1::b, p4); //静态成员的地址固定不变
    printf("c1.c: %p, %p; c2.c: %p, %p\n", &(c1.*p5[0]), &c1.c1, &(c2.*p5[0]), &c2.c1); //成员指针可以对应不同对象的字段
    printf("%d, %d, %d\n", p5[0], p5[1], p5[2]); //成员指针直接输出时的值就是指向的类字段在类中的偏移量

    C2 c3;
    C3 c4;
    c3.**p5; //成员指针可以像类指针一样向下传递使用
    c4.*p5[0];
    int C2::*p7 = p5[0]; //也可以转换为子类成员指针
    // int C3::*p8 = p5; //但不能转换为虚继承的子类成员指针
}

#endif

// #define TEST_2
#ifdef TEST_2
// B站看到的一种利用成员指针访问private的方法
// 头文件
template <typename Member>
class FPrivateAccess
{
public:
    inline static typename Member::Type MemberPtr;
};
// 此处是不能使用模板函数来代替模板类做静态成员MemberPtr的初始化的，因为模板函数类型参数不能接受&FTest::VarA
template <typename Member, typename Member::Type Ptr>
struct FStaticPtrInit // 利用构造函数初始化成员指针
{
    struct FConstructPrivateAccess
    {
        FConstructPrivateAccess()
        {
            FPrivateAccess<Member>::MemberPtr = Ptr;
        }
    };
    inline static FConstructPrivateAccess TriggerConstruct; // 必须为inline，否则指针无意义；static则是让其编译期初始化
};
// 用例，特别的使用成员指针来直接访问FTest中的VarA
class FTest
{
public:
    FTest() { std::cout << "FTest" << std::endl; }

private:
    int VarA = 1;
};

struct FTestAccessVarA
{
    using Type = int FTest::*;
};
// 下面是关键，利用编译期的模板特化来传入平时不可传入的私有成员
template struct FStaticPtrInit<FTestAccessVarA, &FTest::VarA>;
// 自己也能类似的实现
template <typename Member, typename Member::Type Ptr>
class A
{
};
template struct A<FTestAccessVarA, &FTest::VarA>;

int main()
{
    FTest Test;
    FTest *TestPtr = new FTest;
    std::cout << Test.*FPrivateAccess<FTestAccessVarA>::MemberPtr << std::endl
              << TestPtr->*FPrivateAccess<FTestAccessVarA>::MemberPtr;
}
#endif