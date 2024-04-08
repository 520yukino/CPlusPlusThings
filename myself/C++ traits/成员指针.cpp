#include <iostream>
#include <functional>
#define TEST_1
#ifdef TEST_1
struct Test_bind
{
    void t(int n)
    {
        for (; n; n--)
            putchar('t');
        putchar('\n');
    }
    void t()
    {
        std::cout << "a = " << a << std::endl;
    }
    Test_bind *operator->() //重载->，注意必须返回类指针以供默认->运行
    {
        std::cout << "in operator->" << std::endl;
        return this;
    }
    static void f1() {std::cout << "in statci f1" << std::endl;}
    static int b;

private:
    int a = 1;
};
int Test_bind::b = 114514;
//成员指针并非属于某一对象，而是整个类，所以它的功能不太寻常，就好似类的专用函数指针
//函数指针无法指向非静态成员函数，因为这类函数中有隐含的this指针，而成员指针恰恰相反
int main()
{
    Test_bind tb1;
    auto n = std::bind(static_cast<void (Test_bind::*)(int)>(&Test_bind::t), tb1, 3);auto n1 = std::bind(Test_bind::operator->, tb1);
    n(); // 调用t(3)版本
    tb1->t(); //此为调用重载的->，其等效与(tb1.operator->())->t();，注意是对象本身调用运算符而非指针调用，它可以在调用t之前先执行重载->中的内容
    void (Test_bind::*p1)() = Test_bind::t; //有重载时不能auto，无法推断
    (tb1.*p1)(); //和函数指针一样，不能使用重载功能
    auto p2 = &Test_bind::f1; //此为函数指针而非成员指针，如果f1不是static则p2为成员指针，因为静态函数是整个类所有而非某一对象
    auto p3 = &Test_bind::b; //数据成员同样如此
    printf("%p, %p, %p\n", p1, p2, p3); //静态函数的地址固定不变
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