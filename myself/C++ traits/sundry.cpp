#include <iostream>
using namespace std;
template<decltype(auto) N>
class C {
public:
    int a = N;
};
void F1() {cout << "over!" << endl;}
template<typename T, typename... Args>
void F1(const T& a, const Args&... args) 
{
    static int n = sizeof...(args);
    cout << n << ": " << a << ", ";
    F1(args...);
}
int main()
{
    int a1[] {1, 2, 3};
    auto a2 = a1; //对数组auto得到的是一个指针，这也意味着不能定义auto a2[3] = a1，同时不能将a2赋给数组引用int (&a4)[3] = a2;
    auto &a3 = a1; //引用数组
    auto &a4 = a2; //引用指针
    constexpr int b1[10] {3};
    // *begin(b1) = 2; //constexpr当然不能改变值

    int n1 = 1+2;
    int b2[n1] = {1, 2, 3, 4}; //C++的某些编译器继承了C中的变长数组，此处b2就只有3个元素，初始化中的4被忽略，但我们应避免使用此语法
    for (int i: b2) { cout << i << ' '; } cout << endl;

    C<1> x;
    cout << "x.a = " << x.a << endl;
    auto & c1 = "char";
    auto b3 = [a1](int x) {double y = 3.14; return x*a1[1]*y;}; //不能使用函数指针指向lambda
    cout << "b3(7) = " << b3(7) << endl;
    F1(7, 'c', string("just"));
}

decltype(auto) f1(int& a) { return (a); } //带括号会被auto为引用