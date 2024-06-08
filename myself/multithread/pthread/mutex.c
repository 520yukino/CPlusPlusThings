#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
//互斥锁，某线程有和其它线程同时对某些数据进行读写操作时可能发生线程使用的数据未及时更新的现象，这时可以使用互斥锁锁上线程中读写操作的代码段，这样所有拥有该锁的代码段只能同时执行其中一个
//某个互斥量被锁上后，其它所有lock该互斥量的地方都会被阻塞，这种机制也可能会导致死锁，比如：加锁后忘记解锁、重复加锁形成自锁
#define LOOP_NUM 10
int a = 0;
pthread_mutex_t mut;// = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;
//直接赋值初始化锁或调用init函数初始化任选一种，也可以直接不初始化，全局变量会默认为0。注意linux下无法使用这种方式，必须使用初始化函数
//锁可以不初始化，它会有默认值
void *thread_Func1(void *arg)
{
    pthread_mutex_lock(&mut);
    printf("res: %d\n", pthread_mutex_lock(&mut)); //返回EDEADLK==36
    for (int i = 1; i <= LOOP_NUM; i++)
    {
        pthread_mutex_lock(&mut);
        //如果使用trylock则不会阻塞，而是仅返回错误码，用户可以根据返回值知晓互斥量的情况而做出相应响应
        // usleep(1000);
        a++;
        pthread_mutex_unlock(&mut);
        printf("in thread func1, i = %d, a = %d\n", i, a);
    }
    return NULL;
}

void *thread_Func2(void *arg)
{
    for (int i = 1; i <= LOOP_NUM; i++)
    {
        pthread_mutex_lock(&mut);
        // usleep(1000);
        a++;
        pthread_mutex_unlock(&mut);
        printf("in thread func2, i = %d, a = %d\n", i, a);
    }
    return NULL;
}

int main()
{
    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mut, &ma);
    /*
    init可以指定锁的类型，在posix中锁本身和类型参数仅仅只是一些整形：
    PTHREAD_MUTEX_TIMED_NP，这是缺省值，也就是普通锁。当一个线程加锁以后，其余请求锁的线程将形成一个等待队列，并在解锁后按优先级获得锁。这种锁策略保证了资源分配的公平性。
    PTHREAD_MUTEX_RECURSIVE_NP，嵌套锁，允许同一个线程对同一个锁成功获得多次，并通过多次unlock解锁。如果是不同线程请求，则在加锁线程解锁时重新竞争。 
    PTHREAD_MUTEX_ERRORCHECK_NP，检错锁，如果同一个线程请求同一个锁，则返回EDEADLK，否则与PTHREAD_MUTEX_TIMED_NP类型动作相同。这样就保证当不允许多次加锁时不会出现最简单情况下的死锁。
    PTHREAD_MUTEX_ADAPTIVE_NP，适应锁，动作最简单的锁类型，仅等待解锁后重新竞争。
    */
    pthread_t pid;
    pthread_create(&pid, NULL, thread_Func2, NULL);
    pthread_create(&pid, NULL, thread_Func1, NULL);
    pthread_create(&pid, NULL, thread_Func2, NULL);

    pthread_exit(NULL);
    pthread_mutexattr_destroy(&ma);
    pthread_mutex_destroy(&mut);
    //销毁一个互斥锁即意味着释放它所占用的资源，且要求锁当前处于开放状态。由于在Linux中，互斥锁并不占用任何资源，因此LinuxThreads中的pthread_mutex_destroy()除了检查锁状态以外（锁定状态则返回EBUSY）没有其他动作。
    return(0);
}
