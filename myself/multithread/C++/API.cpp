#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <iomanip>
#include <mutex>
#include <future>
#include <vector>
using namespace std;

mutex mut;
//读写锁在C++中变为了共享锁shared_mutex

void print_time()
{ // 打印当前时间
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);
    cout << "now is: " << put_time(localtime(&in_time_t), "%Y-%m-%d %X") << endl;
}

void sleep_thread()
{
    this_thread::sleep_for(chrono::seconds(1)); // 内部调用nanosleep，sleep_until则等待至指定时间点
    mut.lock();
    cout << "[thread " << this_thread::get_id() << "], is waking up" << endl;
    mut.unlock();
}

void yield_thread(int i)
{
    // if (i == 3)
    //     for (size_t i = 0; i < 100; i++)
    //         this_thread::yield(); //让出cpu，重新进行线程调度，但并不是阻塞，只是弹出资源而后立即进入就绪态和其它线程继续竞争
    mut.lock();
    cout << "[thread " << this_thread::get_id() << "], print: " << i << endl;
    mut.unlock();
}

void init()
{
    static int n;
    cout << "in init(), n = " << ++n << endl;
}

void worker(once_flag *flag)
{
    //call_once在once_flag标志下让init函数在被多次调用时只运行一次
    call_once(*flag, init);
}

int main()
{
    vector<thread> vt;
    print_time();
    thread ta1(sleep_thread);
    for (size_t i = 0; i < 5; i++)
        vt.push_back(thread(yield_thread, i));
    for (size_t i = 0; i < 5; i++)
        vt[i].detach();
    ta1.join();
    print_time();

    once_flag flag; //想要标志有效的作用在call_once上，必须保证标志唯一且存在
    thread tb1(worker, &flag);
    thread tb2(worker, &flag);
    thread tb3(worker, &flag);
    // this_thread::sleep_for(chrono::milliseconds(500));
    tb1.join();
    tb2.join();
    tb3.join();

    {
        lock_guard<mutex> g(mut);
        //互斥锁守卫，具有RAII功能的智能互斥锁，用互斥锁锁初始化即可在构造函数中加锁，析构中解锁
        unique_lock<mutex> u1(mut, try_to_lock), u2(mut, adopt_lock);
        //unique_lock<mutex>则类似unique_ptr，拥有锁的所有权，可以通过移动函数转让所有权，并且可以构造中调用或者类方法中调用mutex中所有的操作；同时参数2可以指定多种不同初始状态的锁，try可以尝试加锁但不阻塞，adopt则假设锁已经被锁上
    }
    
    return 0;
}