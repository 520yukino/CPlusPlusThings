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
    cout << n << ":" << a << ", ";
    F1(args...);
}
int main()
{
    int a1[] {1, 2, 3};
    auto a2 = a1; //对数组auto得到的是一个指针，这也意味着不能定义auto a2[3] = a1，同时不能将a2赋给数组引用
    auto& a3 = a1;
    constexpr int b1[10] {3};
    //*begin(a3) = 2; //constexpr当然不能改变值
    int a4 = 1+2;
    int b2[a4] = {1,2,3,4};
    C<1> x;
    cout << x.a << endl;
    auto & c1 = "char";
    auto b3 = [a1](int x) {double y = 3.14; return x*a1[1]*y;};
    cout << b3(7) << endl;
    F1(7, 'c', string("just"));
}

decltype(auto) f1(int& a) {return (a);} //带括号会被auto为引用