#include "ThreadPool.h"
#include <stdio.h>

ThreadPool::ThreadPool(int thread_num, int max_requests) : 
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

ThreadPool::~ThreadPool() {
    destroy();
    m_stop = true;
}

bool ThreadPool::append(BaseRequest* request) {
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

void ThreadPool::destroy() {
    delete [] m_threads;
    pthread_mutex_destroy(&m_mutex);
    sem_destroy(&m_sem);
}

void* ThreadPool::worker(void* args) {
    ThreadPool* pool = (ThreadPool*)args;
    pool->run();
    return pool;
}

void ThreadPool::run() {
    while (!m_stop) {
        sem_wait(&m_sem);
        pthread_mutex_lock(&m_mutex);
        if (m_request_queue.empty()) {
            pthread_mutex_unlock(&m_mutex);
            continue;
        }
        BaseRequest* request = m_request_queue.front();
        m_request_queue.pop();
        pthread_mutex_unlock(&m_mutex);
        if (!request) {
            continue;
        }
        printf("tid %u do a task\n", (unsigned int)pthread_self());
        request->process();
        delete request;
    }
}
