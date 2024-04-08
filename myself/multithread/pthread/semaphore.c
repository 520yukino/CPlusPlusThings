#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
//信号量机制相当于将资源数封装了的条件变量机制，依旧使用消费生成模型，信号量的资源数量是自带的而无需像条件变量一样需要手动设置条件，当资源为0时造成阻塞
//如果资源数量大于1，则依旧需要互斥量来使得线程串行运行，因为生产者和消费者可能同时读写资源造成崩溃；互斥锁应该放在信号锁中间，否则会造成死锁
sem_t sempro, semcus;
pthread_mutex_t mut = PTHREAD_MUTEX_DEFAULT;
struct Node
{
    int num;
    struct Node *next;
};
struct Node *head = NULL;
size_t count = 0;
const size_t MAX_COUNT = 10;
// count为货物的剩余数，n为每个线程的生成或消费数，sum是总的消费或生产数

void *Func_pro(void *arg)
{
    size_t n = 0;
    static size_t sum = 0;
    while (1)
    {
        struct Node *node = (struct Node *)malloc(sizeof(struct Node));
        sem_wait(&sempro); //消耗一个sempro资源数，如果没有资源则阻塞，trywait则返回错误码而不阻塞，timedwait同cond_timedwait，不过只有资源为0时才会阻塞
        pthread_mutex_lock(&mut); //注意需要在内部加互斥锁防止并行读写数据
        node->next = head;
        head = node;
        node->num = ++n;
        printf("producer: thread id %d, num %d, sum %d, count %d\n", pthread_self(), n, ++sum, ++count);
        pthread_mutex_unlock(&mut);
        sem_post(&semcus); //给semcus增加一个资源，使其中一个wait中的消费者解锁
        usleep(rand() % RAND_MAX * 1);
    }
}

void *Func_cus(void *arg)
{
    size_t n = 0;
    static size_t sum = 0;
    while (1)
    {
        sem_wait(&semcus);
        pthread_mutex_lock(&mut);
        struct Node *temp = head;
        head = temp->next;
        printf("customer: thread id %d, num %d, sum %d, count %d, n %d\n",
               pthread_self(), temp->num, ++sum, --count, ++n);
        pthread_mutex_unlock(&mut);
        sem_post(&sempro);
        free(temp);
        usleep(rand() % RAND_MAX * 1);
    }
}

int main()
{
    int proresource = 10, cusresource = 0; //生产者和消费者的初始资源数，注意消费者必须为0，因为链表一开始就是为空的
    sem_init(&sempro, 0, proresource);
    sem_init(&semcus, 0, cusresource);
    //参数2为0是多线程，非0是多进程，参数3是初始资源个数，两个信号量的资源数之和即为总资源数
    srand(time(NULL));
    int procount = 5, cuscount = 5;
    pthread_t propt[procount], cuspt[cuscount];
    for (size_t i = 0; i < procount; i++)
    {
        pthread_create(&propt[i], NULL, Func_pro, NULL);
    }
    // usleep(1000);
    for (size_t i = 0; i < cuscount; i++)
    {
        pthread_create(&cuspt[i], NULL, Func_cus, NULL);
    }
    sleep(2);
    return 0;
}