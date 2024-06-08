#include <iostream>
#include <vector>
using namespace std;

class Test
{
public:
    Test();
    Test(const Test &t);
    ~Test();
};

Test::Test()
{
    cout << "object created\n";
}
Test::Test(const Test &t)
{
    cout << "copy\n";
}
Test::~Test()
{
    cout << "~object deleted\n";
}

//打印整个vector的元素及其容量
void PrintVec(auto rem, const std::vector<int>& c)
{
    for (std::cout << rem; const int el: c) //C++20的范围for可以有初始化语句
        std::cout << el << ' ';
    cout << "| " << c.capacity();
    std::cout << '\n';    
}

/* 主要测试vector的内存情况以及对应的方法 */
int main()
{
    vector<Test> vec(2);
    cout << "after create, before clear ... " << endl;
    vec.clear();
    cout << "after clear ... " << endl;
    cout << vec.size() << endl;
    cout << vec.capacity() << endl;
    vector<Test> v1;
    vector<Test>(vec).swap(vec); //复制构造一个临时vector来swap掉原来的vector，而后自动析构，这样vec就不会再有以前的空间大小而是临时量的空间大小
    cout << v1.capacity() << endl;
    cout << vec.capacity() << endl; //vec当然就和空数组一样为空了

    vector<int> c = {1, 2, 3};
    PrintVec("The vector holds: ", c);
    c.resize(5); //resize会改变size大小，即填充或截断vec。无指定初始化的resize会调用默认构造，所以必须保证元素有默认构造才行
    PrintVec("After resize up to 5: ", c);
    c.resize(2);
    PrintVec("After resize down to 2: ", c);
    c.resize(6, 4);
    PrintVec("After resize up to 6(initializer = 4): ", c);

    return 0;
}