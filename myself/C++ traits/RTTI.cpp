#include <typeinfo>
#include <iostream>
using namespace std;
void f1(void)
{
    std::putchar('G');
}
int main()
{
    int a = 2;
    int & b = a;
    std::puts("you jiba who?");
    void (* fun)(void) = f1;
    fun();
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