#include <iostream>
#include <string>
#include <functional>
#include <vector>
using namespace std;
/*f1的第一个版本在没有规定第二版时必须存在，因为当三版递归到0个参数时会由于无匹配的f1而出错
即参数T & t必须存在，有第二版则不需要，但如果用户调用了f1()无参版则又会出错*/
void f1() {}
template <typename T>
void f1(T &t)
{
    cout << t << endl;
}
template <typename T, class... A>
void f1(T &t, A &...a)
{
    cout << t << ", ";
    f1(a...);
}
// 使用C++17中的if constexpr和sizeof...新语法，但如果调用无参版则依旧要有空参数版本
void f2() {}
template <typename T, class... A>
void f2(T &t, A &&...a)
{
    if constexpr (sizeof...(a) > 0) //如果可变参已经为空，则不执行
    {
        cout << t << ", ";
        f2(a...);
    }
    else
        cout << t << endl;
}

template <typename T, typename F>
T f3(T a, F f)
{
    return f(a);
}

int main(int argc, char *argv[])
{
    int a1 = 12;
    double a2 = 3.14;
    string a3 = "cnm";
    f1<int, double, string>(a1, a2, a3);
    f1();
    f2(a1, a2, a3);
    f2();
    function<int(int)> b[2] = {[](int a){ return a * a; },
                               [](int a){ return a + a; }};
    // function包装器可以使模板函数在多个拥有同一种特征标的函数中只使用一个实例化
    cout << f3(20, b[0]) << endl
         << f3(18, b[1]) << endl;

    return 0;
}
