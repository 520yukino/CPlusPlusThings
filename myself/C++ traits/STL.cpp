#include <iostream>
#include <valarray>
#include <initializer_list>
#include <functional>
using namespace std;

using ld = initializer_list<double>;
double average(ld& list)
{
    double sum = 0;
    for(auto i: list) sum += i;
    return sum/list.size();
}

template <typename T>
void use_f(T a, function<T(T)> f) {}

class C1
{
private:
    int a;
public:
    C1() {}
    int operator()(int n1, int n2)
    {
        cout << "in operator()\n";
        return n1*n2;
    }
};

int main()
{
    puts("*** 1 ***"); //数组的一些操作
    valarray<int> a1(2, 40);
    valarray<int> a2 {1,2,3,4};
    for(auto i: a1) cout << i; cout << endl;
    //范围for如果不加&则是复制的实例，加了&则为引用实例，还可以加const表示常量
    a1[slice(1, 10, 2)] = 7; /*库中使用slice类初始化slice_array类，而后operator[]返回slice_array对象
    3个参数分别是起始下标、元素个数、步长跨度，slice_array重载了赋值运算符可以批量赋值*/
    for(auto i: a1) cout << i; cout << endl;

    puts("*** 2 ***"); //initializer_list的用法
    ld l1 = {2, 4, 8};
    cout << average(l1) << endl;

    puts("*** 3 ***"); //函数包装器
    C1 c;
    auto b = bind(C1::operator(), &c, a1[0], placeholders::_1); //占位符表示绑定函数的哪些参数需要在调用时给出，其它参数则被绑定
    //此处直接使用C1::operator()，成员函数其实是类中的方法绑定到一个对象上，所谓的this指针就是第一个传入的对象参数
    cout << b(a1[1], 99, 1) << endl; //占位符只会让这里的参数1有效的绑定到函数的参数2，但依旧可以写上更多的参数，只是无效化而已
    use_f<double>(a2[2], [](double a) {return a*a;});
}