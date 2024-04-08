#include <iostream>
#include <thread>
#include <unistd.h>
#include <memory>
using namespace std;

class Test {
public:
    Test() {
        std::cout << "Test()" << std::endl;
    }
    Test(int a) {
        std::cout << "Test(int a), a=" << a << std::endl;
    }
    ~Test() {
        std::cout << "~Test()" << std::endl;
    }
};

void Func1();
void Func2();

int main()
{
    make_shared<int>(7);
    { //包含在块中测试析构
        auto p1 = new Test;
        shared_ptr<Test> sp1(p1);
        cout << "sp1: " << sp1.use_count() << endl;
        // shared_ptr<Test> sp2(p1); //如果重复使用原始指针初始化智能指针，则其计数器将失效，因为会创建多个计数器，退出时析构多次
        shared_ptr<Test> sp2(sp1);
        cout << "sp2: " << sp2.use_count() << endl;
        shared_ptr<int> sp3(new int(22), [](int*) { cout<<"YES!\n"; });
        //可以指定shared_ptr的deleter，析构时会传入指针来调用此函数
    }
    putchar('\n');

    Func1();
    putchar('\n');
    Func2();
}

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
    shared_ptr<A> pa_;
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
    shared_ptr<A> pa_;
    B2()
    {
        cout << "B2 create\n";
    }
    ~B2()
    {
        cout << "B2 delete\n";
    }
};

void Func1()
{
    shared_ptr<A> pa(new A());
    shared_ptr<B1> pb1(new B1());
    shared_ptr<B2> pb2(new B2());
    cout << "pa: " << pa.use_count() << ", " //1
         << "pb1: " << pb1.use_count() << ", " //1
         << "pb2: " << pb2.use_count() << endl; //1
    pa->pb_1 = pb1; //已经有一个pb指向了B1对象，现在再让pa中的pb_1指向B1对象则会使B1对象的计数器为2
    pa->pb_2 = pb2; //但pb_2使用了weak_ptr，它不会改变计数器，在需要使用此指针的时候再使用lock方法返回shared_ptr即可
    pb1->pa_ = pa; //退出时对象无法被智能释放
    pb2->pa_ = pa;
    cout << "pa: " << pa.use_count() << ", " //3
         << "pb1: " << pb1.use_count() << ", " //2
         << "pb2: " << pb2.use_count() << endl; //1
} //退出时pb2能被释放，但pb1不能，pb1.pa_也就不能销毁，连锁造成pa无法释放

void Observe(std::weak_ptr<int> wptr)
{
    if (auto sptr = wptr.lock()) { //lock会返回shared_ptr，但如果
        std::cout << "in Observe, value: " << *sptr << std::endl;
    } else {
        std::cout << "in Observe, wptr lock fail" << std::endl;
    }
}

void Func2() //测试weak_ptr转变为shared_ptr的条件
{
    std::weak_ptr<int> wptr;
    {
        auto sptr = std::make_shared<int>(111);
        wptr = sptr;
        Observe(wptr);  // sptr 指向的资源没被释放，wptr 可以成功提升为 shared_ptr
    }
    Observe(wptr);  // sptr 指向的资源已被释放，wptr 无法提升为 shared_ptr
}