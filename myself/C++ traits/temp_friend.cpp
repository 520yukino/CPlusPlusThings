#include <iostream>
using namespace std;

template <typename T>
void f1(T&);

template <typename T>
class A
{
public:
    A(int a = 1) : a(a) { b++; }
    ~A() {}
    friend void f1<>(A<T> &); //约束模板友元函数，每个类实例对应一个函数实例。因为它本质是模板函数的具体化，所以需要在类前面先提供模板函数
    template <typename Ta>
    friend void f2(Ta &); // 非约束，是指整个模板函数都是类的友元。它的实例化不与类实例化相绑定，而是根据使用此函数时的传参类型来实例化
    template <typename Tb>
    void f3(Tb &); //模板方法
private:
    int a = 1;
    static T b;
};

template <typename T>
T A<T>::b = 0;

template <typename T>
void f1(T &t)
{
    cout << "a = " << t.a << ", b = " << T::b << endl;
}

template <typename Ta>
void f2(Ta &t)
{
    cout << "in friend template function f2(), a = " << t.a << endl;
}

template <typename T>
template <typename Tb>
void A<T>::f3(Tb &t)
{
    cout << "in class template function f3(), a = " << t.a << endl;
}

int main()
{
    A<double> ca;
    f1(ca);
    A<int> cb[4]{-1, 4, 5, -100};
    for (auto i : cb)
        f1(i);
    f2(cb[3]); // 友元可以访问私有量
    // ca.f3(cb[3]); //f3中无法访问t.a，因为t是传参对象而非调用此方法的对象本身
}
