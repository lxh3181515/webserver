#ifndef __WEBSERVER_THREADPOOL_H
#define __WEBSERVER_THREADPOOL_H

#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <exception>
#include "BaseRequest.h"

class ThreadPool
{
public:
    ThreadPool(int thread_num = 4, int max_requests = 10000);

    ~ThreadPool();

    /* 往请求队列添加任务 */
    bool append(BaseRequest* request);

private:
    /* 工作线程运行函数 */
    static void* worker(void* args);

    /* 资源释放 */
    void destroy();

    void run();

private:
    int m_thread_number; /* 线程池中的线程数 */

    int m_max_requests; /* 请求队列允许最大的请求数 */

    std::queue<BaseRequest*> m_request_queue; /* 请求队列 */

    pthread_t* m_threads; /* 线程数组 */

    pthread_mutex_t m_mutex; /* 互斥锁 */

    sem_t m_sem; /* 任务信号量 */

    bool m_stop; /* 是否停止线程 */
};



#endif
