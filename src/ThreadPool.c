#include "ThreadPool.h"

#include <stdio.h>  //用于输入输出函数
#include <stdlib.h> //用于内存分配
#include <unistd.h> //提供对 POSIX 操作系统 API 的访问，如 sleep
#include <string.h> //用于字符串处理函数

// 工作者线程执行函数
void *worker(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg; // 接受一个 void* 类型的参数，将其转换为 ThreadPool* 类型
    while (1)
    { // 工作线程持续循环，直到收到停止信号
        Task task;

        // 获取任务
        pthread_mutex_lock(&pool->lock);
        while (pool->count == 0 && !pool->stop)
        {
            pthread_cond_wait(&pool->cond, &pool->lock); // 如果任务队列为空且线程池未停止，线程就等待条件变量的信号
        }

        if (pool->stop)
        {
            pthread_mutex_unlock(&pool->lock); // 如果线程池需要停止，释放锁并退出循环
            break;
        }

        // 取出任务
        task = pool->taskQueue[pool->head]; // 从任务队列中获取任务，更新头指针和任务计数，随后解锁
        pool->head = (pool->head + 1) % pool->taskQueueCapacity;
        pool->count--;
        pthread_mutex_unlock(&pool->lock);

        // 执行任务
        task.threadTask(task.args); // 调用任务函数，传入相应的参数
    }

    return NULL; // 线程结束时返回 NULL
}

// 初始化线程池,固定线程数量，队列动态增加存储全部的消息队列
ThreadPool *threadPoolCreate(int numThreads)
{
    int i;
    ThreadPool *pool = malloc(sizeof(ThreadPool)); // 分配内存给一个新的 ThreadPool 结构体

    pool->taskQueueCapacity = 16;                                     // 初始容量
    pool->taskQueue = malloc(sizeof(Task) * pool->taskQueueCapacity); // 设置任务队列的初始容量为 16，并分配相应的内存
    pool->taskQueueSize = 0;
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pool->stop = 0; // head、tail 和 count 初始化为 0，stop 设置为 0（表示未停止）

    pool->threads = malloc(sizeof(pthread_t) * numThreads); // 根据用户指定的线程数量分配线程数组
    pthread_mutex_init(&pool->lock, NULL);                  // 设置互斥锁和条件变量以便线程间同步
    pthread_cond_init(&pool->cond, NULL);

    for (i = 0; i < numThreads; i++)
    {
        pthread_create(&pool->threads[i], NULL, worker, pool); // 根据指定数量创建工作线程，每个线程执行 worker 函数
    }

    return pool; // 函数结束时返回创建的线程池指针
}

// 添加任务到线程池 接受线程池和要添加的任务函数及其参数。加锁以保护共享数据
void threadPoolEnqueue(ThreadPool *pool, void (*threadTask)(void *), void *args)
{
    pthread_mutex_lock(&pool->lock);

    // 扩展任务队列
    if (pool->count == pool->taskQueueCapacity)
    {
        pool->taskQueueCapacity *= 2; // 如果任务队列已满，双倍扩展队列的容量
        pool->taskQueue = realloc(pool->taskQueue, sizeof(Task) * pool->taskQueueCapacity);
    }

    // 添加任务,将新任务添加到队列的尾部，并更新 tail 和 count
    pool->taskQueue[pool->tail].threadTask = threadTask;
    pool->taskQueue[pool->tail].args = args;
    pool->tail = (pool->tail + 1) % pool->taskQueueCapacity;
    pool->count++;

    pthread_cond_signal(&pool->cond); // 发送信号通知有新任务，并解锁以允许其他线程访问
    pthread_mutex_unlock(&pool->lock);
}

// 停止线程池,加锁，设置停止标志，广播通知所有工作线程停止，并解锁
void threadPoolDestroy(ThreadPool *pool)
{
    int i;

    pthread_mutex_lock(&pool->lock);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->lock);

    for (i = 0; i < pool->taskQueueSize; i++)
    {
        pthread_join(pool->threads[i], NULL); // 使用 pthread_join 等待所有工作线程结束
    }

    // 释放任务队列、工作线程数组和线程池本身的内存
    free(pool->taskQueue);
    free(pool->threads);
    free(pool);
}