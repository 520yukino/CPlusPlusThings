#include <iostream>
#include <typeinfo>
using namespace std;

template <typename T>
void put_l_or_r(T &t)
{
    cout << "lvalue" << endl;
}
template <typename T>
void put_l_or_r(T &&t)
{
    cout << "rvalue" << endl;
}

template <typename T>
void testfunc(T &&v) //模板右值引用只能转为左值或右值，不能按值传递，因为没有办法直接去掉&&，最多用&顶替&&
{
    put_l_or_r(v); // 右值引用的形参做其它函数的实参时会被转变为左值引用！
    put_l_or_r(std::forward<T>(v));
    put_l_or_r(std::move(v));
}
void f1(int &&v) //非模板右值引用形参不能将左值作为实参！而为模板时&&的含义是universal reference，即可以接受任何方式的引用
{
    put_l_or_r(v);
    put_l_or_r(std::forward<int&&>(v));
    put_l_or_r(std::move(v));
    // put_l_or_r(std::forward<int&>((int&&)v)); //编译出错，forward中有static_assert
}

int main()
{
    int x = 520;
    testfunc(114514);
    testfunc(x);
    testfunc(std::forward<int>(x));
    testfunc(std::move(x));
    // int &r1 = x, &&r2 = static_cast<int & &&>(r1); //不能使用引用的引用，这进一步说明forward
    return 0;
}