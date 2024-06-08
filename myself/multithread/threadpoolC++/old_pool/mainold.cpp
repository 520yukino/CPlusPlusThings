#include "threadpoolold.h"
#include <string>
#include <stdlib.h>
using std::string;
using tt = long long;

tt Func_task1(int a, string s)
{
	printf("Func_task(), num = %d, threadID = %ld, string = \"%s\"\n", a, pthread_self(), s.c_str());
	// sleep(1);
	tt r = 0;
	for (size_t i = 0; i < 5000000; i++) //给cpu上点强度
	{
		r += rand();
	}
	printf("Func_task() over, num = %d\n", a);
	return r;
}

int main()
{
	srand(time(nullptr));
	int threadmin = 2, threadmax = 16, tasknum = 50;
    threadpool tp(threadmin, threadmax, true);
	std::future<tt> fut[tasknum+1];
	for (int i = 1; i <= tasknum; i++) //future为返回值
	{
		fut[i] = tp.AddWork(Func_task1, i, "ABC");
		usleep(20000);
	}
	for (int i = 1; i <= tasknum; i++)
	{
		printf("future: %d, r = %lld\n", i, fut[i].get());
	}
	while (tp.TaskNum()) //等待任务数为0
	{
		sleep(1);
	}
	tp.Shutdown();

	// if (tp.Run())
	// {
		
	// }

	puts("Over");
    return 0;
}