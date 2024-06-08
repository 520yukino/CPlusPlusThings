#include "threadpool.h"
#include <string>
#include <stdlib.h>
using std::string;
using tt = long long;

tt Func_task1(int a, string s)
{
	printf("Func_task(), num = %d, threadID = %ld, string = \"%s\"\n", a, pthread_self(), s.c_str());
	// sleep(1);
	tt r = 0;
	for (size_t i = 0; i < 5e6; i++) //给cpu上点强度
	{
		r += rand();
	}
	printf("Func_task() over, num = %d\n", a);
	return r;
}

int main()
{
	srand(time(nullptr));
	int threadmin = 2, threadmax = 16, tasknum1 = 50, tasknum2 = 25, futnum = tasknum1-30;
    threadpool tp(threadmin, threadmax, true);
	std::future<tt> fut[tasknum1+tasknum2+1];
	for (int i = 1; i <= tasknum1; i++) //future为返回值
	{
		fut[i] = tp.AddWork(Func_task1, i, "ABC");
		usleep(20000);
	}
	for (int i = 1; i <= futnum; i++) //阻塞接受返回值
	{
		printf("future: %d, r = %lld\n", i, fut[i].get());
	}
	// while (tp.TaskNum()) //等待任务数为0
	// {
	// 	sleep(1);
	// }
	tp.Shutdown();
	puts("*************************\n    Shutdown and Run!\n*************************");

	if (tp.Run())
	{
		for (int i = tasknum1+1; i <= tasknum1+tasknum2; i++) //future为返回值
		{
			fut[i] = tp.AddWork(Func_task1, i, "cum");
			usleep(20000);
		}
		for (int i = futnum+1; i <= tasknum1+tasknum2; i++)
		{
			printf("future: %d, r = %lld\n", i, fut[i].get());
		}		
	}

	puts("******************\n    main over!\n******************");
    return 0;
}