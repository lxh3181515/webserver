#include "ChatRoomTask.h"
#include <fcntl.h>
#include <assert.h>

std::map<int, ChatRoomTask::client_data> ChatRoomTask::sm_clients;

int ChatRoomTask::sm_client_num = 0;

int ChatRoomTask::sm_listen = -1;

int ChatRoomTask::sm_epfd = -1;

void setNonBlock(int fd)
{
    int option = fcntl(fd, F_GETFL) | O_NONBLOCK;
    fcntl(fd, F_SETFL, option);
}

void addfd(int efd, int fd, uint32_t events, bool oneshot)
{
    epoll_event event;
    event.events = events | EPOLLET;
    if (oneshot)
        event.events |= EPOLLONESHOT;
    event.data.fd = fd;
    int ret = epoll_ctl(efd, EPOLL_CTL_MOD, fd, &event);
    if (ret == -1)
    {
        if (errno == ENOENT) // 文件描述符未注册
        {
            epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);
            setNonBlock(fd);
        }
        else
        {
            if (ret < 0)
            {
                perror("Error epoll control");
            }
        }
    }
}

ChatRoomTask::~ChatRoomTask() {

}

void ChatRoomTask::setup(int listen, int epfd) {
    sm_listen = listen;
    sm_epfd = epfd;
}

void ChatRoomTask::process() {
    /* 客户端关闭请求 */
    if (m_task == TASK_CLOSE)
    {
        auto pos = sm_clients.find(m_fd);
        if (pos != sm_clients.end())
        {
            sm_clients.erase(pos);
        }
        close(m_fd);
        sm_client_num--;
        epoll_ctl(sm_epfd, EPOLL_CTL_DEL, m_fd, NULL);
        printf("a client left\n");
    }

    /* 接收数据 */
    else if (m_task == TASK_RECV)
    {
        printf("in recv\n");
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
            epoll_ctl(sm_epfd, EPOLL_CTL_DEL, m_fd, NULL);
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
                addfd(sm_epfd, iter->first, EPOLLOUT, true);
            }
        }
        addfd(sm_epfd, m_fd, EPOLLIN | EPOLLRDHUP, true); /* 重置ONESHOT */
    }

    /* 发送数据 */
    else if (m_task == TASK_SEND)
    {
        printf("in send\n");
        if (sm_clients.at(m_fd).send_buf)
        {
            send(m_fd, sm_clients.at(m_fd).send_buf, strlen(sm_clients.at(m_fd).send_buf), 0);
            sm_clients.at(m_fd).send_buf = nullptr;
            printf("sended\n");
        }
        addfd(sm_epfd, m_fd, EPOLLIN | EPOLLRDHUP, true);
    }
}


