#include "threadpoolold.h"

threadpool::threadpool(int tmin, int tmax, bool run)
{
	this->tmin = tmin < 0 ? 0 : tmin;
	this->tmax = tmax < 1 ? 1 : tmax;
	shutdown = true;
	if (run) //run为真则启动线程池
		Run();
}

threadpool::~threadpool()
{
	Shutdown();
}

//启动线程池
bool threadpool::Run()
{
	twait = 0;
	tdestroy = 0;
	if (shutdown) //处于关闭状态
	{
		shutdown = false;
		for (size_t i = 0; i < tmin; i++)
		{
			workerv.emplace_back(Worker, std::ref(*this));
		}
		manager = std::thread(Manager, std::ref(*this));
		return true;
	}
	else
		return false;
}
#include <iostream>
//关闭线程池，可重启
bool threadpool::Shutdown()
{
	if (shutdown) //已经关闭则不能再次关闭
		return false;
	printf("in Destroy(), talive %ld, twait %d\n", workerv.size(), twait);
	shutdown = true;
	manager.join();
	mut.lock();
	condworker.notify_all();
	mut.unlock();
	for (auto &i: workerv)
	{
		std::cout << "in Destroy(), id " << i.get_id() << std::endl;
		i.join();
	}
	printf("workerv size: %lu\n", workerv.size());
	workerv.clear();
	return true;
}

//工作者，消费者
void threadpool::Worker(threadpool& tp)
{
	printf("worker, id "); std::cout << std::this_thread::get_id() << std::endl;
    std::function<void()> func;
	std::unique_lock<std::mutex> lock(tp.mut, std::defer_lock); //配合条件变量
	while (1) //不断执行任务，直到关停，这里的判断是为了迎接还在执行任务的工人
	{
		lock.lock();
		while (tp.taskq.empty() || tp.shutdown) ///队列为空消费者阻塞，或者需要退出
		{
			if (tp.shutdown)
				return;
			printf("pthread_cond_wait 1, id "); fflush(stdout); std::cout << std::this_thread::get_id() << std::endl;
            tp.twait++;
			tp.condworker.wait(lock);
            tp.twait--;
			printf("pthread_cond_wait 2, id "); fflush(stdout); std::cout << std::this_thread::get_id() << std::endl;
			if (tp.tdestroy > 0) //根据管理者的给定减少数来自杀
			{
				tp.tdestroy--;
				printf("destroy worker 1, alive = %ld, id ", tp.workerv.size()); std::cout << std::this_thread::get_id() << std::endl;
				return;
			}
		}
        //取出并执行任务
		func = tp.taskq.front();
		tp.taskq.pop();
		lock.unlock();
		func();
	}
	// printf("destroy worker 2, alive = %ld, id ", tp.workerv.size());
}

//管理者
void threadpool::Manager(threadpool& tp)
{
	int alive, max = tp.tmax, min = tp.tmin; //忙线程数，总工人数，线程最大、最小值
	size_t i;
	while (!tp.shutdown) //不断控制工人，直到关停
	{
		usleep(200000); //检测间隔时间
		printf("|manager|\n");
		tp.mut.lock();
		alive = tp.workerv.size();
		if (tp.taskq.size() > alive * JUDGE_TIMES && alive < max) //任务多于线程*JUDGE_TIMES且线程小于最大值则添加工人
		{
			for (i = 0; i < ADD_NUM && alive < max; i++, alive++)
			{
				tp.workerv.emplace_back(Worker, std::ref(tp));
				printf("add worker, alive = %d\n", alive+1);
			}
		}
		//线程小于等待线程*JUDGE_TIMES且线程大于最小值则减少工人，注意减少人数不一定为DESTROY_NUM
		for (i = 0; alive < tp.twait * JUDGE_TIMES && alive > min && i < DESTROY_NUM; i++, alive--)
		{
			printf("tdestroy++\n");
			tp.tdestroy++; //需要减少的工人数量+1
			tp.condworker.notify_one(); //唤醒可能处在wait的消费者，让他自杀，注意杀死的实际数量取决于tdestroy
			tp.workerv.back().detach(); //先分离线程，而后销毁，子线程依旧可以独立运行至自我销毁
			tp.workerv.pop_back();
		}
		tp.mut.unlock();
	}
	printf("exit manager\n"); fflush(stdout);
}

//获取twait个数
int threadpool::WaitNum() const
{
    return twait;
}

//获取tlive个数
int threadpool::AliveNum() const
{
    return workerv.size();
}

//获取qsize数
int threadpool::TaskNum() const
{
    return taskq.size();
}

//清空任务队列
void threadpool::ClearTask()
{
	while (!taskq.empty())
	{
		taskq.pop();
	}
}
