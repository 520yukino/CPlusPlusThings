#include <iostream>
#include <type_traits>
/*
    decay的作用(下面的T、U均为decay_equiv中的):
    1、若T为“U的数组”或“U的数组的引用”类型，则 decay<T>::type为U*
    2、若T为函数类型F或者它的引用，则decay<T>::type为std::add_pointer<F>::type
    3、否则，decay<T>::type为std::remove_cv<std::remove_reference<T>::type>::type
*/
//decay_equiv通过比较decay<T>::type和U来判定decay后的T与原始U是否相同，借此表明decay的作用
template <typename T, typename U>
struct decay_equiv: std::is_same<typename std::decay<T>::type, U>::type
{};

int main()
{
    std::cout << std::boolalpha
              << decay_equiv<int, int>::value << '\n'                // 情况3
              << decay_equiv<int &, int>::value << '\n'              // 情况3
              << decay_equiv<int &&, int>::value << '\n'             // 情况3
              << decay_equiv<const int &, int>::value << '\n'        // 情况3
              << decay_equiv<int[2], int *>::value << '\n'           // 情况1
              << decay_equiv<int(int), int (*)(int)>::value << '\n'; // 情况2
}
