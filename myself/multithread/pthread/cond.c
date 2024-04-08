#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
// 锁是让线程自行锁住运行，其它有该锁的地方无法别阻塞，但条件变量是阻塞调用wait的线程，让其它线程调用解锁函数来接触阻塞，这两者的目的完全不同
// 条件变量模拟生产消费过程，条件变量需要互斥量配合使用放在临界区中，wait时会将传入的互斥量解锁以解除其它线程的阻塞，signal则解除wait处的阻塞使线程转为就绪态并让其具有抢互斥锁的资格
pthread_cond_t cond;
pthread_mutex_t mut;
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
        pthread_mutex_lock(&mut);
        while (count == MAX_COUNT) // 库存已满，等待消费者消费并发出信号
        {
            printf("producer wait, thread id %d\n", pthread_self());
            pthread_cond_wait(&cond, &mut); // wait是等待唤醒信号唤醒，timedwait是自身定时唤醒
            //注意wait只能放在lock里面，因为wait醒来后并争夺到cpu时会对mut重新加锁，如果放外面就死锁了
        }
        node->next = head;
        head = node;
        node->num = ++n;
        printf("producer: thread id %d, num %d, sum %d, count %d\n", pthread_self(), n, ++sum, ++count);
        pthread_mutex_unlock(&mut);
        pthread_cond_signal(&cond); // signal唤醒一个线程，broadcast唤醒所有
        //唤醒函数放在锁里锁外都可以，区别不大
        usleep(rand() % RAND_MAX * 1);
    }
}

void *Func_cus(void *arg)
{
    size_t n = 0;
    static size_t sum = 0;
    while (1)
    {
        pthread_mutex_lock(&mut);
        while (!head) // 最好使用while循环判断条件变量，因为唤醒并不意味着条件一定合理，可能出现虚假唤醒的情况。比如在生产者中使用broadcast全体唤醒，这时多个消费者竞争互斥锁，但可能消费者数量大于库存数，消费者争先恐后取货使得头指针为空，而后程序访问该指针导致崩溃。有了循环条件判断，前一个消费者取完后，后一个消费者需要判断是否还有库存
        {
            printf("customer wait, thread id %d\n", pthread_self());
            pthread_cond_wait(&cond, &mut);
        }
        struct Node *temp = head;
        head = temp->next;
        printf("customer: thread id %d, num %d, sum %d, count %d, n %d\n",
               pthread_self(), temp->num, ++sum, --count, ++n);
        pthread_mutex_unlock(&mut);
        pthread_cond_broadcast(&cond);
        free(temp);
        usleep(rand() % RAND_MAX * 1);
    }
}

int main()
{
    pthread_mutex_init(&mut, NULL);
    pthread_cond_init(&cond, NULL);
    srand(time(NULL));
    size_t procount = 5, cuscount = 3; //生成者和消费者数量
    pthread_t propt[procount], cuspt[cuscount];
    for (size_t i = 0; i < procount; i++)
    {
        pthread_create(&propt[i], NULL, Func_pro, NULL);
    }
    usleep(200000);
    for (size_t i = 0; i < cuscount; i++)
    {
        pthread_create(&cuspt[i], NULL, Func_cus, NULL);
    }
    sleep(1);
    return 0;
}