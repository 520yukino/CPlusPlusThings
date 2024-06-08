#include <iostream>
#include <thread>
#include <unistd.h>
#include <mutex>
using namespace std;

mutex mtx[2];
void thread_1()
{
    mtx->lock(); // 锁可以使得多个调用该函数的线程只运行被锁住的那一个
    for (size_t i = 0; i < 5; i++)
    {
        cout << "Child Thread 1" << endl;
        usleep(200000);
    }
    mtx->unlock();
}

void thread_2(int x)
{
    mtx[1].lock();
    for (size_t i = 0; i < 5; i++)
    {
        cout << "Child Thread 2, x = " << x << endl;
        usleep(200000);
    }
    mtx[1].unlock();
}

void thread_3(int &x)
{
    mtx[1].lock();
    for (size_t i = 0; i < 5; i++)
    {
        cout << "Child Thread 3, x = " << ++x << endl;
        usleep(200000);
    }
    mtx[1].unlock();
}

int main()
{
    // thread zero;
    int a1 = 10, a2 = 20, a3 = 30;
    cout << thread::id() << endl //直接输出id则会在<<内部判断到此为空id，输出non-executing语句
         << this_thread::get_id() << endl //get_id内部就是调用pthread_self
         << hash<thread::id>()(thread::id()) << endl
         << pthread_self() << endl; //任一进程的主线程id从来都是从1开始
    
    thread first(thread_1); //开启线程，调用：thread_1()
    thread second(thread_2, a2);
    thread third(thread_3, ref(a3));
    //注意！如果传入thread的线程函数的参数是引用参数，则需要确保此参数保持合法，因为如果引用参数在线程执行中被清除，那么引用将变为ub。所以要么直接使用按值传递，要么使用ref包装引用参数，也可以使用bind来包装函数参数
    
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
