#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <future>
//部分摘抄自cppreference
#if 1
void f(int n1, int n2, int n3, const int& n4, int n5)
{
    std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << n4 << ' ' << n5 << '\n';
}
 
int g(int n1)
{
    return n1;
}
 
struct Foo
{
    void print_sum(int n1, int n2)
    {
        std::cout << n1 + n2 << '\n';
    }
 
    int data = 10;
};
 
int main()
{
    using namespace std::placeholders;  // for _1, _2, _3...
 
    std::cout << "1) argument reordering and pass-by-reference: ";
    int n = 7;
    // (_1 and _2 are from std::placeholders, and represent future
    // arguments that will be passed to f1)
    auto f1 = std::bind(f, _2, 42, _1, std::cref(n), n);
    n = 10;
    f1(1, 2, 1001); // 1 is bound by _1, 2 is bound by _2, 1001 is unused
                    // makes a call to f(2, 42, 1, n, 7)
 
    std::cout << "2) achieving the same effect using a lambda: ";
    n = 7;
    auto lambda = [&ncref = n, n](auto a, auto b, auto /*unused*/)
    {
        f(b, 42, a, ncref, n);
    };
    n = 10;
    lambda(1, 2, 1001); // same as a call to f1(1, 2, 1001)
 
    std::cout << "3) nested bind subexpressions share the placeholders: ";
    auto f2 = std::bind(f, _3, std::bind(g, _3), _3, 4, 5);
    f2(10, 11, 12); // makes a call to f(12, g(12), 12, 4, 5);
 
    std::cout << "4) bind a RNG with a distribution: ";
    std::default_random_engine e;
    std::uniform_int_distribution<> d(0, 10);
    auto rnd = std::bind(d, e); // a copy of e is stored in rnd
    for (int n = 0; n < 10; ++n)
        std::cout << rnd() << ' ';
    std::cout << '\n';
 
    std::cout << "5) bind to a pointer to member function: ";
    Foo foo;
    auto f3 = std::bind(&Foo::print_sum, &foo, 95, _1);
    f3(5);
 
    std::cout << "6) bind to a mem_fn that is a pointer to member function: ";
    auto ptr_to_print_sum = std::mem_fn(&Foo::print_sum);
    auto f4 = std::bind(ptr_to_print_sum, &foo, 95, _1);
    f4(5);
 
    std::cout << "7) bind to a pointer to data member: ";
    auto f5 = std::bind(&Foo::data, _1);
    std::cout << f5(foo) << '\n';
 
    std::cout << "8) bind to a mem_fn that is a pointer to data member: ";
    auto ptr_to_data = std::mem_fn(&Foo::data);
    auto f6 = std::bind(ptr_to_data, _1);
    std::cout << f6(foo) << '\n';
 
    std::cout << "9) use smart pointers to call members of the referenced objects: ";
    std::cout << f6(std::make_shared<Foo>(foo)) << ' '
              << f6(std::make_unique<Foo>(foo)) << '\n';
}

#else
void f(int& n1, int& n2, const int& n3)
{
    std::cout << "In function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';
    ++n1; // increments the copy of n1 stored in the function object
    ++n2; // increments the main()'s n2
    // ++n3; // compile error
}
 
int main()
{
    int n1 = 1, n2 = 2, n3 = 3;
    std::packaged_task<void()> bound_f(std::bind(f, n1, std::ref(n2), std::cref(n3)));
    //由于bind需要储存所有绑定的函数参数，所以n1会被复制而非引用，必须使用ref引用包装来包装变量
    //packaged_task类似function的函数包装器，但具有get_future方法，利用future储存函数的返回值
    n1 = 10;
    n2 = 11;
    n3 = 12;
    std::cout << "Before function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';
    bound_f();
    std::cout << "After function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';

    
}
#endif