#include <iostream>
#include <vector>
#include <string>
#include <functional>
using namespace std;

void getMinMax(vector<int> &number, function<void()> &printer)
{
    int min = number.front();
    int max = number.front();
    for (int i : number)
    {
        if (i < min)
        {
            min = i;
        }
        if (i > max)
        {
            max = i;
        }
    }
    //函数指针只能在lambda没有任何捕获参数时才能使用，因为捕获的参数实际上被当做类成员变量了，没有成员变量也就无需传入this指针
    printer = [min = move(min), &max]() mutable { // 通过lambda按值传递使得main中依旧可以使用min和max的值，此为闭包
        cout << "min:" << --min << endl;
        cout << "max:" << max << endl; //引用变量在外部用是ub
    }; // 必须添加mutable才能让按值捕获的实例为非const，而按引用默认为非const
}

template <typename T>
void Func1(T t) { cout << "in Func1(T t)\n"; }
template <typename T>
void Func1(T *t) { cout << "in Func1(T* t)\n"; }
void Func2() { cout << "in Func2()\n"; }

int main(int argc, char const *argv[])
{
    function<void()> printer;
    vector numbers{0, 2, 5, -7};
    getMinMax(numbers, printer);
    printer();
    cout << "sizeof(printer) = " << sizeof(printer) << endl; // function会用堆内存拷贝传入的函数的信息，也可以是bind的返回值或lambda，这些东西都不会改变function对象的大小

    void (*func)() = Func2;
    Func1(func); // 函数指针无论使用&还是*还是本身都会调用T*版本

    auto fun = [](int i) -> int // 如果不指定返回值类型则会以第一个return的类型作为返回类型，且后续不相同的类型将会报错
    {
        if (i % 2 == 0)
            return true;
        else
            return 11.14;
    };

    return 0;
}
