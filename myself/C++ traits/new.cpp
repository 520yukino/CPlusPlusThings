#include <iostream>

class MyClass
{
public:
    MyClass()
    {
        std::cout << "MyClass()" << std::endl;
    }
    MyClass(int a)
    {
        std::cout << "MyClass(int a), " << a << std::endl;
    }
    MyClass(int a, int b)
    {
        std::cout << "MyClass(int a, int b), " << a << ", " << b << std::endl;
    }
    ~MyClass()
    {
        std::cout << "~MyClass" << std::endl;
    }
};

int main()
{
    int arraySize = 5;
    // 使用new[]分配对象数组的内存，注意初始化的手法
    MyClass *myArray = new MyClass[arraySize]{1, {2, 3}}; //可以使用()或空白来表示不做初始化
    // 释放内存
    delete[] myArray;
    return 0;
}
