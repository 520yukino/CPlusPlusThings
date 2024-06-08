#include <typeinfo>
#include <iostream>
using namespace std;

#if 0

void f1(void)
{
    std::putchar('G');
}
int main()
{
    int a = 2;
    int * c1 = &a;
    void * c2 = (c1);
    cout << "\nc2 = " << c2 << ", c2+1 = " << c2+1 << endl;
    c1 = static_cast<int *>(c2);

    union U { int a; double b; } u = {3};
    void* x = &u;                        // x's value is "pointer to u"
    double* y = static_cast<double*>(x); // y's value is "pointer to u.b"
    char* z = static_cast<char*>(x);     // z's value is "pointer to u"
    cout << "x=" << x << ", y=" << *y << ", z=" << int(*z);
    y = (double *)c1;
    cout << ", int(a)->double=" << *y << endl;
    cout << typeid(f1).name() << endl;
}

#else
/* typeid对于类继承多态的类型识别本质上是通过虚函数表来计算的，所以如果基类没有虚函数，则无法正确识别指向的子类类型，而只能识别为引指的本身类型 */
class A{
private:
    int a;
};

class B :public A{
public:
    virtual void f(){ cout << "HelloWorld\n"; }
private:
    int b;
};

class C :public B{
public:
    virtual void f(){ cout << "HelloWorld++\n"; }
private:
    int c;
};

int main()
{
    int a = 2;
    cout << typeid(a).name() << endl;
    A objA;
    cout << typeid(objA).name() << endl;
    B objB;
    cout << typeid(objB).name() << endl;
    C objC;
    cout << typeid(objC).name() << endl;

    puts("*** 1 ***");
    
    B *p1 = new C();
    //打印出class C，B有虚函数
    cout << typeid(*p1).name() << endl;
    
    A *p2 = new B();
    //打印出class A而不是class B，因为A没有虚函数
    cout << typeid(*p2).name() << endl;

    A &r1 = objC;
    //同理，打印出class A而不是class C
    cout << typeid(r1).name() << endl;

    return 0;
}

#endif