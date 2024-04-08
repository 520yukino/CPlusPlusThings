/**
 * @file decltype.cpp
 * @brief g++ -o decltype decltype.cpp -std=c++11
 * @author 光城
 * @version v1
 * @date 2019-08-08
 */

#include <iostream>
#include <vector>
using namespace std;
/**
 * 泛型编程中结合auto，用于追踪函数的返回值类型
 */
template <typename T>

auto multiply(T x, T y) -> decltype(x * y)
{
    return x * y;
}

int main()
{
    int nums[] = {1, 2, 3, 4};
    vector<int> vec(nums, nums + 4);
    vector<int>::iterator it;

    for (it = vec.begin(); it != vec.end(); it++)
        cout << *it << " ";
    cout << endl;

    using nullptr_t = decltype(nullptr);
    nullptr_t nu;
    int *p = NULL;
    if (p == nu)
        cout << "NULL" << endl;

    typedef decltype(vec.begin()) vectype;

    for (vectype i = vec.begin(); i != vec.end(); i++)
        cout << *i << " ";
    cout << endl;

    /**
     * 匿名结构体
     */
    struct
    {
        int d;
        double b;
    } anon_s;

    decltype(anon_s) as; // 定义了一个上面匿名的结构体

    cout << multiply(11, 2) << endl;

    int a1 = 2, &&a2 = 2;
    decltype(a1) d0;
    decltype((a1)) d00 = a1; // 这里的不同在于()表示求值，这使得其行为类似于函数返回值一样提供了一个左值
    decltype(a1++) d1;       // 非左值也非右值引用则为原类型
    decltype(++a1) d2 = a1;  // 左值，为引用类型
    double d3[3];
    decltype(d3) d4;
    decltype(d4[1]) d5 = d3[2];
    decltype(a2) d6 = char(a2);         // 右值引用，为右值引用类型，只能引用右值
    decltype(multiply(a1, a2)) (*d7)(); // 获取函数的返回值类型，而后进行组合形成其它类型，如这里的函数指针
}
