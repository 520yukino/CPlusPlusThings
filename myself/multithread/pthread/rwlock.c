#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
//读写锁，一把锁可以加读锁或者写锁，只能同时存在一个状态，读锁时其它读锁请求可以通过，并行读，写锁时为串行
//写锁优先级高于读锁，写锁必须全部完成才能轮到读锁，只有多个读锁可以并行，其它情况只要有写锁就会阻塞
int a = 0;
pthread_rwlock_t rwlock;
//直接赋值初始化锁或调用init函数初始化任选一种

void *thread_Func_rd(void *arg)
{
    int m;
    usleep(10);
    for (size_t i = 1; i <= 10; i++)
    {
        pthread_rwlock_rdlock(&rwlock);
        //rd为读锁，wr为写锁
        usleep(100);
        printf("in thread func1, i = %d, a = %d\n", i, a);
        pthread_rwlock_unlock(&rwlock);
        //解锁均为unlock
    }
    return NULL;
}

void *thread_Func_wr(void *arg)
{
    int m; 
    for (size_t i = 1; i <= 10; i++)
    {
        pthread_rwlock_wrlock(&rwlock);
        m = a;
        m++;
        usleep(100);
        a = m;
        printf("in thread func2, i = %d, a = %d\n", i, a);
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

int main()
{
    pthread_rwlockattr_t ma = PTHREAD_RWLOCK_INITIALIZER;
    pthread_rwlock_init(&rwlock, &ma);
    int ptnum1 = 7, ptnum2 = 3; //7个线程读，3个写
    pthread_t p1[ptnum1], p2[ptnum2];

    for (size_t i = 0; i < ptnum1; i++)
    {
        pthread_create(&p1[i], NULL, thread_Func_rd, NULL);
    }
    for (size_t i = 0; i < ptnum2; i++)
    {
        pthread_create(&p2[i], NULL, thread_Func_wr, NULL);
    }

    pthread_exit(NULL);
    pthread_rwlock_destroy(&rwlock);
    return 0;
}
