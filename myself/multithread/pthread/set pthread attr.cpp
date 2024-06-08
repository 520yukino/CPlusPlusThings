#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
using namespace std;

#define NUM_THREADS 5

void *wait(void *t)
{
    int num = *(int *)t;
    sleep(1);
    cout << "in thread -> num: " << num << ", exiting..." << endl;
    pthread_exit(NULL);
    return nullptr;
}

int main()
{
    int res, i, a[NUM_THREADS] = {0};
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    void *status;

    // 初始化并设置线程为分离状态，即不可join的
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    for (i = 0; i < NUM_THREADS; i++)
    {
        a[i] = i;
        res = pthread_create(&threads[i], &attr, wait, &a[i]);
        cout << "main(): creating thread " << threads[i] << endl;
        if (res)
        {
            cout << "Error: unable to create thread! res = " << res << endl;
            exit(-1);
        }
    }

    // 删除属性，并等待其他线程
    pthread_attr_destroy(&attr);
    for (i = 0; i < NUM_THREADS; i++)
    {
        res = pthread_join(threads[i], &status);
        if (res)
        {
            cout << "Warning: unable to join! res = " << res << endl;
            continue;
        }
        cout << "main(): completed thread id:" << threads[i] << ", exiting with status: " << status << endl;
    }
    cout << "main(): program exiting." << endl;
    pthread_exit(NULL);
}