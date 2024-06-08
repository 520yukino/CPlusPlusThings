#if 1
#include <iostream>
using namespace std;
/* ref函数会使用reference_wrapper包装传入的需要包装的引用参数，reference_wrapper内部使用对应类型的指针指向该引用，
需要使用该引用时通过内部的operator _Tp&转换函数返回引用。显然试图包装右值参数是非法的
通常ref包装函数返回的包装器只适用于传递给模板参数，再由模板参数传递给其它非模板原类型参数，如果想要直接使用包装器的隐式转换则可能出现问题 */
void printString(std::string const& s)
{
    std::cout << s << '\n';
}

template<typename T1, typename T2>
bool isless(T1 arg1, T2 arg2)
{
    printString(arg1);
    // reference_wrapper虽然有operator T&()，但此处如果模板参数为string则无法隐式转换，因为string的<重载的参数全是const，而引用返回的右值
    // cout << arg1[0] << arg2[0] << endl;
    // return arg1 < arg2;
    return arg1.operator const string &() < arg2;
}

template<class T>
bool Func1(T t1, int t2)
{
    return t1 == t2;
}

template<class T>
class C1
{
    T a;
public:
    C1(T a): a(a) {}
    operator T&() {return a;}
};

int main()
{
    string s1 = "s1";
    string const s2 = "s2";
    if(isless(cref(s1), "ccc"))
        cout << "test 1\n";
    if(isless(cref(s1), s2))
        cout << "test 2\n";
    if(ref(s1).operator string &() < string("z")) //即使在非泛型中ref也不能隐式转换
        cout << "test 3\n";
    C1<int> c1(1); //自定义的operator T&()可以在模板中隐式转换，这也是合乎语法的，但奇怪的是没有explicit的ref却不行
    cout << Func1(c1, 1) << endl;
    int a;
    auto r1 = ref(a);
    auto r2 = ref(s1);
    cout << Func1(r1, 1) << endl;
    a += r1;
    //r2 = s1+"test";
    //普通的int引用可以隐式转换，但string就不行
    C1<string> c2("c2");
    c2 += s1;
    c2.at(1);
    s1[0];
}

#else
#include <cassert>
#include <functional>
#include <map>
#include <optional>
#include <string_view>

using Map = std::map<std::string_view, int>;
using Opt = std::optional<std::reference_wrapper<Map::value_type>>;

Opt find(Map& m, std::string_view s)
{
    auto it = m.find(s);
    return it == m.end() ? Opt{} : Opt{*it};
}

int main()
{
    Map m{{"A", 1}, {"B", 2}, {"C", 3}};

    if (auto opt = find(m, "C"); opt)
        opt->get().second = 42;
        // std::optional::operator->() returns reference to std::reference_wrapper, then
        // reference_wrapper::get() returns reference to map::value_type, i.e. std::pair

    assert(m["C"] == 42);
}

#endif