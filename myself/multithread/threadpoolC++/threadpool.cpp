#include "threadpool.h"

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
			std::thread t(Worker, std::ref(*this));
			workerm[t.get_id()] = std::move(t);
			// workerm.insert(std::pair<std::thread::id, std::thread>(t.get_id(), std::move(t)));
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
	printf("in Destroy(), talive %ld, twait %d\n", workerm.size(), twait);
	shutdown = true;
	manager.join();
	mut.lock();
	condworker.notify_all();
	mut.unlock();
	printf("workerv size: %lu\n", workerm.size());
	for (auto &i: workerm)
	{
		std::cout << "in Destroy(), id " << i.first << std::endl;
		i.second.join();
	}
	workerm.clear();
	assistm.clear();
	return true;
}

//工作者，消费者
void threadpool::Worker(threadpool& tp)
{
	printf("worker, id "); std::cout << std::this_thread::get_id() << std::endl;
    std::function<void()> func;
	std::unique_lock lock(tp.mut, std::defer_lock); //配合条件变量
	while (1) //不断执行任务，直到关停，这里的判断是为了迎接还在执行任务的工人
	{
		lock.lock();
		while (tp.taskq.empty() || tp.shutdown) ///队列为空消费者阻塞，或者需要退出
		{
			if (tp.shutdown)
				return;
			printf("wait 1, id "); fflush(stdout); std::cout << std::this_thread::get_id() << std::endl;
            tp.twait++;
			tp.condworker.wait(lock);
            tp.twait--;
			printf("wait 2, id "); fflush(stdout); std::cout << std::this_thread::get_id() << std::endl;
			if (tp.tdestroy > 0) //根据管理者的给定减少数来自杀
			{
				tp.tdestroy--;
				tp.assistm[std::this_thread::get_id()] = false;
				tp.condmanager.notify_one();
				printf("destroy worker 1, alive = %ld, id ", tp.workerm.size()); std::cout << std::this_thread::get_id() << std::endl;
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
	std::unique_lock lock(tp.mut, std::defer_lock);
	while (!tp.shutdown) //不断控制工人，直到关停
	{
		usleep(200000); //检测间隔时间
		printf("|manager|\n");
		lock.lock();
		alive = tp.workerm.size();
		if (tp.taskq.size() > alive * JUDGE_TIMES && alive < max) //任务多于线程*JUDGE_TIMES且线程小于最大值则添加工人
		{
			for (i = 0; i < ADD_NUM && alive < max; i++, alive++)
			{
				std::thread t(Worker, std::ref(tp));
				tp.assistm[t.get_id()] = true;
				tp.workerm[t.get_id()] = std::move(t);
				printf("add worker, alive = %d\n", alive+1);
			}
		}
		//线程小于等待线程*JUDGE_TIMES且线程大于最小值则减少工人，注意减少人数不一定为DESTROY_NUM
		if (alive < tp.twait * JUDGE_TIMES && alive > min)
		{
			printf("manager, alive: %ld, wait: %d\n", tp.workerm.size(), tp.twait); fflush(stdout);
			for (i = 0; alive < tp.twait * JUDGE_TIMES && alive > min && i < DESTROY_NUM; i++, alive--)
			{
				printf("tdestroy++\n"); fflush(stdout);
				tp.tdestroy++; //需要减少的工人数量+1
				tp.condworker.notify_one(); //唤醒可能处在wait的消费者，让他自杀，注意杀死的实际数量取决于tdestroy
				tp.condmanager.wait(lock);
			}
			lock.unlock();
			for (auto i = tp.assistm.begin(); i != tp.assistm.end(); ) //检查assistm中的标志位，为false时移除两个map中的元素
			{
				if (!i->second)
				{
					tp.workerm[i->first].join(); //先分离线程，而后销毁，子线程依旧可以独立运行至自我销毁
					tp.workerm.erase(i->first);
					printf("in manager destroy worker, alive = %ld, id ", tp.workerm.size()); std::cout << i->first << std::endl;
					tp.assistm.erase((i++)->first); //注意i的递增决策，移除后递增会段错误
				}
				else
					++i;
			}
		}
		if (lock.owns_lock())
			lock.unlock();
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
    return workerm.size();
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
