#include "ChatRoomTask.h"
#include <fcntl.h>
#include <assert.h>

std::map<int, ChatRoomTask::client_data> ChatRoomTask::sm_clients;

int ChatRoomTask::sm_client_num = 0;

int ChatRoomTask::sm_listen = -1;

int ChatRoomTask::sm_epfd = -1;

pthread_mutex_t ChatRoomTask::sm_mutex;



ChatRoomTask::~ChatRoomTask() {

}

void ChatRoomTask::setup(int listen, int epfd) {
    sm_listen = listen;
    sm_epfd = epfd;

    if (pthread_mutex_init(&sm_mutex, nullptr) != 0) {
        throw std::exception();
    }
}

void ChatRoomTask::process() {
    /* 客户端关闭请求 */
    if (m_task == TASK_CLOSE)
    {
        if (pthread_mutex_lock(&sm_mutex) != 0) {
            throw std::exception();
        }
        auto pos = sm_clients.find(m_fd);
        if (pos != sm_clients.end())
        {
            sm_clients.erase(pos);
        }
        close(m_fd);
        sm_client_num--;
        epoll_ctl(sm_epfd, EPOLL_CTL_DEL, m_fd, NULL);
        pthread_mutex_unlock(&sm_mutex);
        printf("a client left\n");
    }

    /* 接收数据 */
    else if (m_task == TASK_RECV)
    {
        if (pthread_mutex_lock(&sm_mutex) != 0) {
            throw std::exception();
        }
        memset(sm_clients.at(m_fd).recv_buf, '\0', sizeof(sm_clients.at(m_fd).recv_buf));
        int bytes_num = recv(m_fd, sm_clients.at(m_fd).recv_buf, sizeof(sm_clients.at(m_fd).recv_buf) - 1, 0);
        if (bytes_num < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return;
            }
            sm_client_num--;
            close(m_fd);
            delfd(sm_epfd, m_fd);
            return;
        }
        else
        {
            printf("get %d bytes of client data \"%s\" from %d\n", bytes_num, sm_clients.at(m_fd).recv_buf, m_fd);
            for (auto iter = sm_clients.begin(); iter != sm_clients.end(); iter++)
            {
                if (m_fd == iter->first)
                    continue;
                iter->second.send_buf = sm_clients.at(m_fd).recv_buf;
                printf("in recv modify EPOLLOUT of fd:%d\n", iter->first);
                modfd(sm_epfd, iter->first, EPOLLOUT, true);
            }
        }
        printf("in recv modify EPOLLIN | EPOLLRDHUP of fd:%d\n", m_fd);
        modfd(sm_epfd, m_fd, EPOLLIN | EPOLLRDHUP, true); /* 重置ONESHOT */
        pthread_mutex_unlock(&sm_mutex);
    }

    /* 发送数据 */
    else if (m_task == TASK_SEND)
    {
        if (pthread_mutex_lock(&sm_mutex) != 0) {
            throw std::exception();
        }
        if (sm_clients.at(m_fd).send_buf)
        {
            send(m_fd, sm_clients.at(m_fd).send_buf, strlen(sm_clients.at(m_fd).send_buf), 0);
            sm_clients.at(m_fd).send_buf = nullptr;
        }
        printf("in send modify EPOLLIN | EPOLLRDHUP of fd:%d\n", m_fd);
        modfd(sm_epfd, m_fd, EPOLLIN | EPOLLRDHUP, true);
        pthread_mutex_unlock(&sm_mutex);
    }
}


