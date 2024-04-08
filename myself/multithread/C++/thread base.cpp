#include <iostream>
#include <thread>
#include <unistd.h>
#include <mutex>
using namespace std;

mutex mtx[2];
void thread_1()
{
    for (size_t i = 0; i < 5; i++)
    {
        cout << "Child Thread 1" << endl;
        usleep(200000);
    }
}

void thread_2(int &x)
{
    mtx->lock(); // 锁可以使得多个调用该函数的线程只运行被锁住的那一个
    for (size_t i = 0; i < 5; i++)
    {
        cout << "Child Thread 2, x = " << x << endl;
        usleep(200000);
    }
    mtx->unlock();
}

int main()
{
    // thread zero;
    int a1 = 11, a2 = 12;
    cout << thread::id() << endl //直接输出id则会在<<内部判断到此为空id，输出non-executing语句
         << this_thread::get_id() << endl //get_id内部就是调用pthread_self
         << hash<thread::id>()(thread::id()) << endl
         << pthread_self() << endl; //任一进程的主线程id从来都是从1开始
    thread first(thread_1);     // 开启线程，调用：thread_1()
    thread second(thread_2, ref(a1));
    thread third(thread_2, ref(a2));
    ///注意！！thread需要确保函数所需参数保持合法，想象一下，如果引用参数在线程执行中被清除，那么引用将变为ub，所以要么不使用引用参数，要么使用ref包装引用参数，或者bind复制包装参数
    first.join();
    second.detach();
    for (int i = 0; i < 3; i++)
    {
        usleep(200000);
        std::cout << "main thread\n";
    }
    third.join();
    return 0;
}
