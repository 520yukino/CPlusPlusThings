#include <stdio.h>
#include <windows.h>

long long a = 0; //func2中的同步自增量
int tdcount; //线程调用次数
CRITICAL_SECTION cs; //用户模式的锁
HANDLE mutex, sem, event; //内核模式的互斥锁、信号量、事件对象
#define HD_COUNT MAXIMUM_WAIT_OBJECTS //WaitForMultipleObjects函数等待句柄的最大数目，不应超过这个数目，否则函数会出错
#define ErrorPuts(s) do { \
    printf("%s ErrorCode %lu\n", s, GetLastError()); \
    exit(-1); \
} while(0)

DWORD WINAPI Func1(LPVOID param)
{
    int n = *(int *)param;
    for (int i = 0; i < n; i++)
    {
        puts("in Func1()");
        Sleep(100);
    }
    return 0;
}

DWORD Func2(LPVOID param)
{
    char *s = (char *)param;
    if (!strcmp(s, "cs")) //用户CRITICAL_SECTION
        EnterCriticalSection(&cs); //获取cs，类似lock
    else if (!strcmp(s, "mutex")) //内核mutex
        WaitForSingleObject(mutex, INFINITE); //使用对象等待函数获取互斥锁，类似timedlock，mutex是auto_reset模式的内核对象，调用wait函数后mutex可以从signaled状态变为non-signaled状态，进而使得其它线程的wait函数阻塞
    else if (!strcmp(s, "sem")) //内核semaphore
        WaitForSingleObject(sem, INFINITE); //与mutex的获取方法一致，但这里的信号量的值大于0时为signaled状态，不阻塞，为0时才阻塞，成功返回时信号量-1
    else if (!strcmp(s, "event")) //内核event
        WaitForSingleObject(event, INFINITE); //同样等待event置为signaled状态，而后根据创建时所用函数的参数2来判断是否自动转变状态为non
    for (int i = 0; i < 1e6; i++)
        a++;
    if (!strcmp(s, "cs"))
        LeaveCriticalSection(&cs); //释放cs，类似unlock
    else if (!strcmp(s, "mutex"))
        ReleaseMutex(mutex); //释放mutex
    else if (!strcmp(s, "sem"))
        ReleaseSemaphore(sem, 1, NULL); //释放sem，参数2为添加的信号量值(超过最大值则不增加并返回-1)，修改前的信号量值的地址
    else if (!strcmp(s, "event"))
        SetEvent(event); //将event重新设置为signaled状态，相反的，ResetEvent则是设置为non状态。此函数不能随意使用，例如它不能操作线程句柄的状态
    tdcount++;
    return 0;
}

int main()
{
    int param_int = 5; //函数传参
    const char *param_str;
    DWORD tid, dw; //线程ID，wait函数返回值
    HANDLE hd, hds[HD_COUNT]; //线程句柄，句柄集合

    //线程的创建和等待
    //创建线程(还可用_beginthreadex)，参数分别为：线程安全信息、栈大小(0为默认大小)、函数指针、函数参数、指定创建后行为(0为立即执行，CREATE_SUSPENDED为挂起)、线程ID的地址(ID不同于句柄，句柄唯一于进程，而ID唯一于整个内核)。成功时返回句柄，失败返回NULL
    hd = CreateThread(NULL, 0, Func1, (LPVOID)&param_int, 0, &tid);
    Sleep(200);
    puts("Sleep() end");
    //阻塞等待线程终止，参数为句柄和超时时间(毫秒)，成功返回事件信息，失败返回WAIT_FAILED，超时返回WAIT_TIMEOUT。线程状态有终止(signaled)和未终止(non-signaled)，如果线程终止，此函数返回WAIT_OBJECT_0。对于此函数，只要对象处于signaled状态则成功返回，否则阻塞
    if ((dw = WaitForSingleObject(hd, INFINITE)) == WAIT_FAILED)
        ErrorPuts("WaitForSingleObject() error!");
    printf("thread state: %s, handle: %lu, tid: %lu\n", (dw == WAIT_OBJECT_0) ? "signaled" : "non-signaled", hd, tid);

    //测试用户同步中的CRITICAL_SECTION
    param_str = "cs";
    InitializeCriticalSection(&cs); //初始化用户锁
    for (int i = 0; i < HD_COUNT; i++) //循环创建线程
    {
        hds[i] = CreateThread(NULL, 0, Func2, (LPVOID)param_str, 0, NULL);
    }
    //阻塞等待所有线程终止，参数分别为：线程数目、线程句柄集合、是否等待所有线程终止(为真则阻塞等待所有线程，为假则只等待1个线程)、超时时间
    if ((dw = WaitForMultipleObjects(HD_COUNT, hds, TRUE, INFINITE)) == WAIT_FAILED)
        ErrorPuts("WaitForMultipleObjects() error!");
    DeleteCriticalSection(&cs); //销毁用户锁，释放资源
    printf("CRITICAL_SECTION: a = %lld, count = %d, dw = %d\n", a, tdcount, dw);

    //测试内核同步中的mutex
    param_str = "mutex";
    //创建mutex对象，参数为：安全信息设置、是否获取锁(为真则直接锁住，为假则不锁)、互斥对象的名称
    mutex = CreateMutex(NULL, FALSE, NULL);
    for (int i = 0; i < HD_COUNT; i++)
    {
        hds[i] = CreateThread(NULL, 0, Func2, (LPVOID)param_str, 0, NULL);
    }
    if ((dw = WaitForMultipleObjects(HD_COUNT, hds, TRUE, INFINITE)) == WAIT_FAILED)
        ErrorPuts("WaitForMultipleObjects() error!");
    CloseHandle(mutex); //销毁mutex，释放资源，此函数可以销毁句柄，包括各种内核模式同步对象
    printf("Mutex: a = %lld, count = %d\n", a, tdcount);

    //测试内核同步中的sem
    param_str = "sem";
    //创建sem对象，参数为：安全信息设置、信号量的初始值、信号量的最大值、互斥对象的名称
    sem = CreateSemaphore(NULL, 1, 1, NULL);
    for (int i = 0; i < HD_COUNT; i++)
    {
        hds[i] = CreateThread(NULL, 0, Func2, (LPVOID)param_str, 0, NULL);
    }
    if ((dw = WaitForMultipleObjects(HD_COUNT, hds, TRUE, INFINITE)) == WAIT_FAILED)
        ErrorPuts("WaitForMultipleObjects() error!");
    CloseHandle(sem); //销毁sem
    printf("Semaphore: a = %lld, count = %d\n", a, tdcount);

    //测试内核同步中的event，event不同于其它，它可以设置manual_reset模式，使得wait后不主动进入non状态
    param_str = "event";
    //创建event对象，参数为：安全信息设置、是否为manual模式(真为manual，假为auto，这是指调用Wait函数时不自动将信号置为non状态，必须调用Reset才行)、是否为signaled状态、对象名称
    event = CreateEvent(NULL, FALSE, TRUE, NULL);
    for (int i = 0; i < HD_COUNT; i++)
    {
        hds[i] = CreateThread(NULL, 0, Func2, (LPVOID)param_str, 0, NULL);
    }
    if ((dw = WaitForMultipleObjects(HD_COUNT, hds, TRUE, INFINITE)) == WAIT_FAILED)
        ErrorPuts("WaitForMultipleObjects() error!");
    CloseHandle(event); //销毁event
    printf("Event: a = %lld, count = %d\n", a, tdcount);

    return 0;
}