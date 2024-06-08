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
        usleep(300000);
        printf("in children thread, i = %d\n", i);
        pthread_testcancel(); //此为测试是否需要强制终止，如果没有这个函数则不会强制终止
        //此处需要注意win和linux下的不同点，win下的cancel函数必须配合此函数来完成子线程的退出；而linux下还可以在阻塞处直接退出，例如输入输出函数，这里遇见printf后不打印并直接退出
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
    sleep(1);
    // 获取已终止线程的返回值，此处的join对强制终止和一般退出的线程的反映是一致的
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
