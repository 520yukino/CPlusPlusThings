#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
// 线程执行的函数
void *thread_Fun(void *arg)
{
    printf("新建线程开始执行\n");
    for (size_t i = 1; i <= 10; i++)
    {
        usleep(500000);
        printf("in children thread, i = %d\n", i);
        pthread_testcancel(); //此为测试是否需要强制终止，如果没有这个函数则不会强制终止
    }
    return NULL;
}

int main()
{
    pthread_t myThread;
    void *mess;
    int res;
    // 创建 myThread 线程
    res = pthread_create(&myThread, NULL, thread_Fun, NULL);
    if (res != 0)
    {
        printf("线程创建失败\n");
        return 0;
    }
    sleep(1);
    // 向 myThread 线程发送 Cancel 信号，信号只是发送过去，并非强制性的终止线程
    res = pthread_cancel(myThread);
    if (res != 0)
    {
        printf("终止 myThread 线程失败\n");
        return 0;
    }
    // 获取已终止线程的返回值，注意此处的join依旧会等待子线程，除非被强制终止
    res = pthread_join(myThread, &mess);
    if (res != 0)
    {
        printf("等待线程失败\n");
        return 0;
    }
    // 如果线程被强制终止，其返回值为 PTHREAD_CANCELED，如果正常终止，则为0
    if (mess == PTHREAD_CANCELED)
    {
        printf("myThread 线程被强制终止\n");
    }
    else
    {
        printf("error\n");
    }
    // printf("mess = %lld, PTHREAD_CANCELED = %lld\n", mess, PTHREAD_CANCELED);
    return 0;
}
