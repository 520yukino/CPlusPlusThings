#include <iostream>
#include <vector>
#include <string>
#include <functional>
using namespace std;
/* lambda的本质是带有operator()重载的类，捕获就是相同类型的类成员，依靠构造函数传入捕获参数初始化，function容纳lambda的本质是容纳lambda对象，也就是函数对象 */

//获取具备输出数组中的最大最小值的function
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
    void (*fp)(int) = [](int a){};
    auto lam = [min](int a){}; //只能用auto来声明lambda变量，此变量的内部类型依旧是lambda而非函数指针
    // fp = lam; //不行
    printer = [min = move(min), &max]() mutable { // 通过lambda按值传递使得main中依旧可以使用min和max的值，此为闭包。这里使用了初始化捕获(没啥用)，注意捕获非自动变量会出警告
        cout << "min:" << min-- << endl;
        cout << "max:" << max << endl; //引用变量在外部用是ub
    }; // 必须添加mutable才能让按值捕获的实例为非const，而按引用默认为非const
}

template <typename T>
void Func1(T t) { cout << "in Func1(T t)\n"; }
template <typename T>
void Func1(T *t) { cout << "in Func1(T* t)\n"; }
void Func2() { cout << "in Func2()\n"; }

class lambda_test //lambda生成类的模仿类
{
    public: 
    inline /*constexpr */ int operator()(int a) const
    {
        puts("in lambda_test::operator()(int a)");
        return a;
    }
    
    using retType = int (*)(int);
    inline constexpr operator retType () const noexcept //不明所以的函数。。。作用是返回函数指针
    {
        return __invoke;
    };
    
    private: 
    static inline /*constexpr */ int __invoke(int a)
    {
        return lambda_test{}.operator()(a);
    }
};

/* 杂项 */
void main1()
{
    function<void()> printer;
    vector numbers{0, 2, 5, -7, 1, 13, -4};
    getMinMax(numbers, printer);
    printer();
    cout << "sizeof(printer) = " << sizeof(printer) << endl; // function会用堆内存拷贝传入的函数的信息，也可以是bind的返回值或lambda，这些东西都不会改变function对象的大小

    void (*func)() = Func2;
    Func1(*****func); // 函数指针无论使用&还是*还是本身都会调用T*版本

    auto fun = [](int i) -> int // 如果不指定返回值类型则会以第一个return的类型作为返回类型，且后续不相同的类型将会报错
    {
        if (i % 2 == 0)
            return true;
        else
            return 11.14;
    };

    lambda_test lt;
    lt(2);
}

/* 测试lambda和function容器的关系和区别 */
void main2()
{
    string str = "test";
    printf("String address %p in main, str %s\n", &str, str.c_str());
    auto funca = [str]()
    {
        printf("String address %p (main lambda), str %s\n", &str, str.c_str());
    }; //此处的诡异输出，使用按值捕获时funca无法输出str的内容，但基于funca的其它function对象均可正常输出；而改为引用捕获后funca可以正常输出

    function<void()> funcb = funca; //使用lambda复制构造
    function<void()> funcc;
    funcc = move(funca); //使用lambda移动赋值，事实上lambda的移动和复制是同一效果

    printf("funca\n");
    funca();
    printf("funcb\n");
    funcb();
    printf("funcc\n");
    funcc();

    function<void()> funcd = move(funcc); //function的移动构造，移动后原function不可再调用
    printf("funcd\n");
    funcd();

    function<void()> funce;
    funce = move(funcd); //function的移动赋值，不能使用已经无效的function来移动，如funcc
    printf("funce\n");
    funce();
}

int main()
{
    main1();
    main2();
    return 0;
}

