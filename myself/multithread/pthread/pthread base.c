#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#define NUM_Threads 5
// 利用结构传输多个数据给线程
typedef struct thread_data
{
    int num;
    char message;
} THDATA, *PTHDATA;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 使用宏初始化线程锁

void *SubThread(void *pthreadid)
{
    if (pthreadid == NULL) //传参为空时退出
        return NULL;    
    THDATA tid = *(PTHDATA)(pthreadid); // 绝对不要直接使用传入的指针作为线程的变量，因为如果主线程结束，指针就ub了
    sleep(1);
    pthread_testcancel();
    printf("This is thread %llu, num: %d, info: %c\n", pthread_self(), tid.num, tid.message);
    char *s = (char *)malloc(30);
    sprintf(s, "thread %llu over!", pthread_self());
    // pthread_exit(s); //用在此处和return没什么区别，但在main中可以使得main结束后其它线程继续执行
    return s;
}

int main(void)
{
    pthread_t Pthread[NUM_Threads]; //线程id是系统分配而后通过create第一个参数传出，一般子线程从1开始++
    THDATA para[NUM_Threads];
    for (int i = 0; i < NUM_Threads; i++) //注意pthread_t初始值是不确定的，需要自行初始化
        printf("threadID %llu\n", Pthread[i]);

    puts("*** 1 ***");
    int i, index = 0, ret;
    for (i = 0; i < NUM_Threads; i++)
    {
        para[i].num = i;
        para[i].message = 'A' + i;
        ret = pthread_create(&Pthread[i], NULL, SubThread, &para[i]);
        if (0 != ret)
        {
            printf("Error: 创建线程失败！\n");
            exit(-1);
        }
        printf("main() : create %llu threads\n", Pthread[i]);
    }

    puts("*** 2 ***");
    pthread_detach(Pthread[index]); //detach可以分离子线程，但分离后无法再做join回收处理
    printf("join res: %d\n", pthread_join(Pthread[index], NULL));
    usleep(1000); //最好睡一会再谈子线程退出，因为子线程需要复制传入的参数
    index++;
    printf("cancel thread: %llu, res: %d\n", Pthread[index], pthread_cancel(Pthread[index])); // 此函数可以删除对应线程，但不能删除自己
    //注意如果线程被cancel强行终止，则主线程使用join回收时第二参数res会返回PTHREAD_CANCELED;

    puts("*** 3 ***");
    char **rs = (char **)malloc(sizeof(char *));
    *rs = "no message";
    index++;
    pthread_join(Pthread[index], (void **)rs); //join可以接收返回值
    puts(*rs);
    printf("currency pthread: %d\n", pthread_getconcurrency());

    puts("*** 4 ***");
    pthread_exit(NULL); // 其它线程可以继续执行
    for (int i = 0; !usleep(10000), i < 5; i++) // 删除了main线程后，剩余指令不会再执行
        puts("in main, while(1)...");
    return 0; // 此为终止进程
}
