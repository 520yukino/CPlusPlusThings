#include <iostream>
#include <thread>
#include <unistd.h>
#include <future>
#include <functional>
using namespace std;

class C1
{
private:
    int a;
public:
    C1(int a = 1): a(a) {}
    int operator()(int n1, int n2)
    {
        cout << "in operator()\n";
        return n1*n2;
    }
    static void f1(C1 &c) { cout << "in f1(), a = " << c.a << endl;}
    int f2() {
        cout << "in f2(), a = " << a << endl;
        thread t(C1::operator(), this, 2, -8);
        t.join();
        return 1;
    }
};

int main()
{
    C1 c1(114514);
    c1.C1::operator()(2, -8);
    thread t0(C1::operator(), c1, 2, -8);
    //传入成员函数所需的隐藏对象就能使用成员函数做函数，此处既可以c1也可以&c1，传入的对象被用作this指针
    //这个特性不仅可以用在这里，所有类似使用函数加其参数的地方如bind，都可以这样使用成员函数，当然，也可以直接使用static函数
    thread t1((int (C1::*)())C1::f2, &c1);
    //成员函数指针同样如此
    C1::f1(c1); //静态函数可以直接调用并传入对象
    thread t2(C1::f1, ref(c1)); //当然也可以传入需要函数的地方
    auto r = async(bind(&C1::f2, c1)).get();
    cout << "f2() reture value = " << r << endl;
    t0.join();
    t1.join();
    t2.join();
}