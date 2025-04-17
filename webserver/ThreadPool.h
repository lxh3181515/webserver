#ifndef __WEBSERVER_THREADPOOL_H
#define __WEBSERVER_THREADPOOL_H

#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <exception>


template <typename T>
class ThreadPool
{
public:
    ThreadPool(int thread_num = 2, int max_requests = 10000);

    ~ThreadPool();

    /* 往请求队列添加任务 */
    bool append(T *request);

private:
    /* 工作线程运行函数 */
    static void* worker(void* args);

    /* 资源释放 */
    void destroy();

    void run();

private:
    int m_thread_number; /* 线程池中的线程数 */

    int m_max_requests; /* 请求队列允许最大的请求数 */

    std::queue<T *> m_request_queue; /* 请求队列 */

    pthread_t* m_threads; /* 线程数组 */

    pthread_mutex_t m_mutex; /* 互斥锁 */

    sem_t m_sem; /* 任务信号量 */

    bool m_stop; /* 是否停止线程 */
};

template <typename T>
ThreadPool<T>::ThreadPool(int thread_num, int max_requests) : 
    m_thread_number(thread_num), m_max_requests(max_requests), m_threads(nullptr), m_stop(false)
{
    if (thread_num <= 0 || max_requests <= 0) {
        throw std::exception();
    }

    /* 初始化互斥锁 */
    if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
        throw std::exception();
    }

    /* 初始化信号量 */
    if (sem_init(&m_sem, 0, 0) != 0) {
        pthread_mutex_destroy(&m_mutex);
        throw std::exception();
    }

    /* 初始化线程资源 */
    m_threads = new pthread_t[thread_num];
    if (!m_threads) {
        throw std::exception();
    }

    for (int i = 0; i < thread_num; i++) {
        if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
            destroy();
            throw std::exception();
        }
        if (pthread_detach(m_threads[i])) {
            destroy();
            throw std::exception();
        }
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool() {
    destroy();
    m_stop = true;
}

template <typename T>
bool ThreadPool<T>::append(T *request) {
    if (pthread_mutex_lock(&m_mutex) != 0) {
        destroy();
        throw std::exception();
    }
    if (m_request_queue.size() >= m_max_requests) {
        pthread_mutex_unlock(&m_mutex);
        return false;
    }
    m_request_queue.push(request);
    pthread_mutex_unlock(&m_mutex);
    sem_post(&m_sem);
    return true;
}

template <typename T>
void ThreadPool<T>::destroy() {
    delete [] m_threads;
    pthread_mutex_destroy(&m_mutex);
    sem_destroy(&m_sem);
}

template <typename T>
void* ThreadPool<T>::worker(void* args) {
    ThreadPool* pool = (ThreadPool*)args;
    pool->run();
    return pool;
}

template <typename T>
void ThreadPool<T>::run() {
    while (!m_stop) {
        sem_wait(&m_sem);
        pthread_mutex_lock(&m_mutex);
        if (m_request_queue.empty()) {
            pthread_mutex_unlock(&m_mutex);
            continue;
        }
        T* request = m_request_queue.front();
        m_request_queue.pop();
        pthread_mutex_unlock(&m_mutex);
        if (!request) {
            continue;
        }
        request->process();
    }
}

#endif
