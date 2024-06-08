#include <iostream>
#include <thread>
#include <unistd.h>
#include <memory>
using namespace std;

void Func1();
void Func2();
void Func3();

int main()
{
    puts("*** 1 ***");
    Func1();
    puts("*** 2 ***");
    Func2();
    puts("*** 3 ***");
    Func3();
}

/* make系列制作智能指针的一些特性 */
class Test {
public:
    Test() {
        cout << "Test()" << endl;
    }
    Test(int a) {
        cout << "Test(int a), a=" << a << endl;
    }
    ~Test() {
        cout << "~Test()" << endl;
    }
};

void Func1()
{
    auto a1 = make_shared<int>(10);
    auto a2 = make_unique<shared_ptr<int>>(a1); //制作一个指向指向int的shared_ptr指针的unique_ptr指针
    auto a3 = make_unique<int[][2]>(10); //unique可以指向数组，当然写法上必须传入无界限数组作为模板参数，函数参数才规定数组大小
    cout << "a1 = " << *a1 << ", a2 = " << **a2 << ", sizeof(*a3) = " << sizeof(a3) << endl;
    { //包含在块中测试析构
        auto p1 = new Test;
        shared_ptr<Test> sp1(p1);
        cout << "sp1: " << sp1.use_count() << endl;
        // shared_ptr<Test> sp2(p1); //如果重复使用原始指针初始化智能指针，则其计数器将失效，因为会创建多个计数器，退出时析构多次
        shared_ptr<Test> sp2(sp1);
        cout << "sp2: " << sp2.use_count() << endl;
        shared_ptr<int> sp3(new int(22), [](int *) { cout << "shared_ptr deleter\n"; });
        //可以指定shared_ptr的deleter，析构时会传入指针来调用此函数
    }
}

/* weak_ptr的作用，类之间互相制约时的智能释放问题 */
class B1;
class B2;
class A
{
public:
    shared_ptr<B1> pb_1;
    weak_ptr<B2> pb_2;
    A()
    {
        cout << "A create\n";
    }
    ~A()
    {
        cout << "A delete\n";
    }
};
class B1
{
public:
    shared_ptr<A> pa_1;
    B1()
    {
        cout << "B1 create\n";
    }
    ~B1()
    {
        cout << "B1 delete\n";
    }
};
class B2
{
public:
    shared_ptr<A> pa_1;
    B2()
    {
        cout << "B2 create\n";
    }
    ~B2()
    {
        cout << "B2 delete\n";
    }
};

void Func2()
{
    shared_ptr<A> pa(new A());
    shared_ptr<B1> pb1(new B1());
    shared_ptr<B2> pb2(new B2());
    cout << "pa: " << pa.use_count() << ", " //1
         << "pb1: " << pb1.use_count() << ", " //1
         << "pb2: " << pb2.use_count() << endl; //1
    pa->pb_1 = pb1; //已经有一个pb指向了B1对象，现在再让pa中的pb_1指向B1对象则会使B1对象的计数器为2
    pa->pb_2 = pb2; //但pb_2使用了weak_ptr，它不会改变计数器，在需要使用此指针的时候再使用lock方法返回shared_ptr即可
    pb1->pa_1 = pa;
    pb2->pa_1 = pa;
    cout << "pa: " << pa.use_count() << ", " //3
         << "pb1: " << pb1.use_count() << ", " //2
         << "pb2: " << pb2.use_count() << endl; //1，weak不计数
} //退出时pb_2由于是weak，所以B2能直接因pb2的销毁而释放；但B1不能，因为pb_1任然存在，B1中的pa_1也就不能销毁，连锁造成A无法释放，也就是A和B1互相制约无法释放

/* 测试weak_ptr的lock方法 */
void Observe(weak_ptr<int> wptr)
{
    //weak不能直接解引用，但可以使用lock方法返回shared_ptr实例进而使用；但如果share已经释放(计数器为0)则返回空指针
    if (auto sptr = wptr.lock()) {
        cout << "in Observe, value: " << *sptr << endl;
    } else {
        cout << "in Observe, wptr lock fail" << endl;
    }
}

void Func3()
{
    weak_ptr<int> wptr;
    {
        auto sptr = make_shared<int>(111);
        // shared_ptr<int>p1,p2;p1=sptr;shared_ptr<int>(sptr).swap(p2);cout<<sptr.use_count()<<endl; //复制赋值相当于构造后交换
        wptr = sptr;
        Observe(wptr); //sptr指向的资源还没释放，wptr的lock方法可以成功返回shared_ptr
    }
    Observe(wptr); //sptr指向的资源已被释放，lock返回nullptr
}