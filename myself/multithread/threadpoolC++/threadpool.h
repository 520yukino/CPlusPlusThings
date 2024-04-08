#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <functional>
#include <unistd.h>
#include <future>
//C++版线程池，使用STL构建
//由于管理者删除线程时是让子线程自行竞争退出，所以无法确定删除了哪些线程，因此使用workerm和assistm来联系工作者和管理者
class threadpool
{
    std::queue<std::function<void()>> taskq; //任务队列
    std::map<std::thread::id, std::thread> workerm; //工作者线程，和线程id关联
    std::map<std::thread::id, bool> assistm; //线程id和bool标志关联，true为正常运行，false为已删除，由于设计目的是可以让管理者删除确定的线程，所以只需记录管理者增添的线程
    std::thread manager; //管理者线程
	int tmin, tmax, twait, tdestroy; //线程最小、最大值，等待中的线程，需要销毁的线程
    std::mutex mut; //整个线程池的锁
    std::condition_variable condworker, condmanager; //无任务时阻塞工人线程的条件变量，管理者删除子线程时所需变量
    bool shutdown; //是否关闭，true为关
    //管理者一次性的线程添加数、线程销毁数，管理者判断是否操作时的差距倍数
    static const int ADD_NUM = 2, DESTROY_NUM = 3, JUDGE_TIMES = 2;
public:
    threadpool(int tmin, int tmax, bool run = false);
    ~threadpool();
    //启动线程池
    bool Run();
    //关闭线程池
    bool Shutdown();
    //添加任务，生产者
    template<typename F, typename... Args>
    auto AddWork(F &&f, Args&&... args) -> std::future<decltype(f(args...))>;
    //获取twait个数
    int WaitNum() const;
    //获取tlive个数
    int AliveNum() const;
    //获取qsize数
    int TaskNum() const;
    //清空任务队列
    void ClearTask();
private:
    //工作者，消费者
    static void Worker(threadpool& tp);
    //管理者
    static void Manager(threadpool& tp);
};

//添加任务，生产者
template<typename F, typename... Args>
auto threadpool::AddWork(F &&f, Args&&... args) -> std::future<decltype(f(args...))>
{
    auto ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(std::bind(f, args...));
	//通过decltype获取传入函数的返回值后与()相组合形成函数类型，packaged_task这个具有返回未来返回值功能的函数包装器，share_ptr则是为了RAII和lambda中的副本
	std::function<void()> func([ptr](){ (*ptr)(); }); //封装为void()，注意这里需要传入指针而非packaged_task本身，因为后续会用到get_future，需要确保lambda内部和调用get_future的packaged_task对象是同一个，而如果按值捕获势必会复制一个对象出来
	std::lock_guard<std::mutex> lock(mut);
	taskq.emplace(func);
	condworker.notify_one();
    printf("task add, num = %ld\n", taskq.size());
	return ptr->get_future();
}

