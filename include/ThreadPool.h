#include <pthread.h>  //用于线程创建、管理和同步

// 参数结构体
typedef struct TaskArgs {
    int epoll_fd;
    int socket_fd;
    char *massage;
} TaskArgs;


// 任务结构体
typedef struct Task {
    void (*threadTask)(void* args); // 任务函数指针
    void* args;                    // 任务参数
} Task;

// 线程池结构体
typedef struct ThreadPool {
    pthread_t* threads;           // 工作线程数组
    Task* taskQueue;              // 任务队列
    int taskQueueSize;            // 任务队列大小
    int taskQueueCapacity;        // 任务队列容量
    int head;                     // 任务队列头
    int tail;                     // 任务队列尾
    int count;                    // 当前任务数量
    pthread_mutex_t lock;         // 互斥锁
    pthread_cond_t cond;          // 条件变量
    int stop;                     // 线程池停止标志
} ThreadPool;

ThreadPool* threadPoolCreate(int numThreads);
void threadPoolEnqueue(ThreadPool *pool, void (*threadTask)(void *), void *args);
void threadPoolDestroy(ThreadPool *pool);