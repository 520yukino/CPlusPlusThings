#include <iostream>
#include <thread>
#include <unistd.h>
#include <future> //std::future std::promise
using namespace std;

int f1(int &n)
{
    for (int i = 0; i < 3; ++i)
    {
        std::cout << "Thread 2 executing\n";
    }
    n = 10;
    return n;
}

void f2(std::promise<int> &promiseObj)
{
    std::cout << "Inside thread: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    promiseObj.set_value(35); //返回值
    // promiseObj.set_value(70); //重复设置会出错
}

int main()
{
    std::thread *t1;
    int n = 0, result = 0;
    t1 = new thread([&]()
                    { result = f1(n); });
    t1->join();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    printf("############\t%d\n", result);
    std::promise<int> promiseObj; //定义promise
    std::future<int> futureObj(promiseObj.get_future()); //绑定future
    std::thread t2(f2, std::ref(promiseObj)); //传入promise
    t2.detach();
    std::cout << futureObj.get() << std::endl; //获取返回值，没有就阻塞
    // std::cout << futureObj.get() << std::endl; //重复获取会出错

    cout << forward<int>((int &&)n) << ' ' << std::is_lvalue_reference<int>::value << endl;
}