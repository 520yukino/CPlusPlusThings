#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#define NUM_Threads 26
// 利用结构传输多个数据给线程
typedef struct thread_data
{
    pthread_t threadid;
    char message;
} THDATA, *PTHDATA;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 使用宏初始化线程锁

void *PrintHello(void *pthreadid)
{
    if (pthreadid == NULL) //传参为空时退出
        return NULL;    
    THDATA tid = *(PTHDATA)(pthreadid); // 绝对不要直接使用传入的指针作为线程的变量，因为如果主线程结束，指针就ub了
    sleep(1);
    pthread_testcancel();
    printf("This is Pthread: %lld; info: %c; pthread_self: %lld\n", tid.threadid, tid.message, pthread_self());
    char *s = (char *)malloc(30);
    sprintf(s, "thread %d over!", tid.threadid);
    // pthread_exit(s); //用在此处和return没什么区别，但在main中可以使得main结束后其它线程继续执行
    // FILE *f = fopen("test.txt", "w");
    // fprintf(f, "file output, thread %d; info: %c\n", tid->threadid, tid->message);
    return s;
}

int main(void)
{
    pthread_t Pthread[NUM_Threads]; //线程id是系统分配而后通过create第一个参数传出，一般子线程从1开始++
    THDATA index[NUM_Threads];
    for (int i = 0; i < NUM_Threads; i++) //注意pthread_t初始值是不确定的，需要自行初始化
        printf("threadID %d\n", Pthread[i]);
    int i, ret;
    pthread_t pid;
    for (size_t i = 0; i < 10; i++)
    {
        pthread_create(&pid, NULL, PrintHello, NULL);
        printf("%d ", pid);
    }
    puts("\n");
    for (i = 0; i < NUM_Threads; i++)
    {
        printf("main() : create %d threads\n", i);
        index[i].threadid = i;
        index[i].message = 'A' + i % 26;
        ret = pthread_create(&Pthread[i], NULL, PrintHello, &index[i]);
        if (0 != ret)
        {
            printf("Error: 创建线程失败！\n");
            exit(-1);
        }
    }

    // pthread_detach(Pthread[NUM_Threads - 1]); //detach可以分离子线程，但分离后无法再做join回收处理
    // printf("join res: %d\n", pthread_join(Pthread[NUM_Threads - 1], NULL));

    /* char **rs = (char **)malloc(sizeof(char *));
    *rs = "no message";
    pthread_join(Pthread[NUM_Threads - 1], (void **)rs); //join可以接收返回值
    puts(*rs); */
    // printf("currency pthread: %d\n", pthread_getconcurrency());

    usleep(10000);      // 最好睡一会在exit，因为子线程需要复制传入的参数
    printf("cancel res: %d\n", pthread_cancel(Pthread[1]));   // 此函数可以删除对应线程，但不能删除自己
    pthread_exit(NULL); // 其它线程可以继续执行
    //注意如果线程被cancel强行终止，则主线程使用join回收时第二参数res会返回PTHREAD_CANCELED;

    for (int i = 0; !usleep(10000), i < 5; i++) // 删除了main线程后，剩余指令不会再执行
        puts("in main, while(1)...");
    return 0; // 此为终止进程
}
