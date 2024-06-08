#include <iostream>
#include <memory>
using namespace std;

class One
{
private:
    size_t n;
    char * c;
    static int count;
    shared_ptr<int> p_i = nullptr; //使用unique_ptr会使得程序无法生成默认的赋值运算符
public:
    One(): n(0), c(nullptr) {cout << "in One(): " << ++count << endl;}
    One(const char * c);
    One(const One& o);
    One(One&& o);
    One& operator=(const One& o) = default;
    ~One();
};

int One::count = 0;

One::One(const char * c)
{
    cout << "in One(const char * c): " << ++count << endl;
    n = sizeof(c);
    this->c = new char[n];
    for (size_t i = 0; i < n; i++)
    {
        this->c[i] = c[i];
    }
}

One::One(const One &o)
{
    cout << "in One(const One &o): " << ++count << endl;
    n = o.n;
    c = new char[n];
    for (size_t i = 0; i < n; i++)
    {
        c[i] = o.c[i];
    }
}

One::One(One &&o)
{
    cout << "in One(One &&o): " << ++count << endl;
    n = o.n;
    c = o.c;
    o.n = 0;
    o.c = nullptr;
    p_i = static_cast<decltype(p_i)&&>(o.p_i);
}

One::~One()
{
    cout << "in ~One(): " << --count << endl;
    delete[] c;
}

void f1(One o)
{
    cout << "in f1()" << endl;
}

int main()
{
    One a1, a2("Hello World!");
    auto a3 = move(a2);
    a1 = a2;
    a1 = move(a2); //const引用的赋值运算符可以同时接受左值和右值
    f1(One()); //创建无名对象并传入函数，正常情况下函数形参应该复制构造一个新的对象，但编译器优化直接使用无名对象
    f1(a1);
}