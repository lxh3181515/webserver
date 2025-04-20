#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <iostream>
#include <cassert>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include "ChatRoomTask.h"
#include "ThreadPool.h"

using namespace std;

#define ERROR_CHECK(x, str) \
    do                      \
    {                       \
        if (x < 0)          \
        {                   \
            perror(str);    \
            assert(0);      \
        }                   \
    } while (0)

#define MAX_EVENT_NUM 1024

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        cout << "usage: " << argv[0] << " ip_address port_number" << endl;
        return -1;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);

    int s = socket(PF_INET, SOCK_STREAM, 0);
    assert(s >= 0);

    sockaddr_in addr;
    bzero(&addr, sizeof(addr)); // 初始化为零
    addr.sin_family = AF_INET;
    inet_aton(ip, &(addr.sin_addr));
    addr.sin_port = htons(port);

    ERROR_CHECK(bind(s, (sockaddr *)&addr, sizeof(addr)),
                "Error binding");

    ERROR_CHECK(listen(s, 5),
                "Error listen");

    int efd = epoll_create(5);
    ERROR_CHECK(efd, "Error create epoll");
    addfd(efd, s, EPOLLIN);

    epoll_event events[MAX_EVENT_NUM];
    ChatRoomTask::setup(s, efd);
    ThreadPool<ChatRoomTask> pool;

    while (1)
    {
        int event_num = epoll_wait(efd, events, MAX_EVENT_NUM, -1);
        ERROR_CHECK(event_num,
                    "Error epoll wait");

        for (int i = 0; i < event_num; i++)
        {
            int fd = events[i].data.fd;
            if ((fd == s) && (events[i].events & EPOLLIN)) // 连接请求
            {
                sockaddr_in addr_client;
                socklen_t addr_client_len = sizeof(addr_client);
                int new_fd = accept(s, (sockaddr *)&addr_client, &addr_client_len);
                if (ChatRoomTask::sm_client_num < MAX_CLIENT_NUM)
                {
                    ChatRoomTask::client_data data;
                    data.addr = addr_client;
                    data.send_buf = nullptr;
                    ChatRoomTask::sm_clients.insert(std::pair<int, ChatRoomTask::client_data>(new_fd, data));
                    ChatRoomTask::sm_client_num++;
                    addfd(efd, new_fd, EPOLLIN | EPOLLRDHUP, true);
                    printf("a client came\n");
                    for (auto iter = ChatRoomTask::sm_clients.begin(); iter != ChatRoomTask::sm_clients.end(); iter++)
                    {
                        printf("%d\n", iter->first);
                    }
                }
                else
                {
                    const char *reply = "too many clents\n";
                    send(new_fd, reply, strlen(reply), 0);
                    close(new_fd);
                }
            }
            else if (events[i].events & EPOLLRDHUP) // 客户端关闭连接
            {
                ChatRoomTask* task = new ChatRoomTask(TASK_CLOSE, fd);
                pool.append(task);
            }
            else if (events[i].events & EPOLLIN) // 接收客户端数据
            {
                printf("detect a EPOLLIN event from %d\n", fd);
                ChatRoomTask* task = new ChatRoomTask(TASK_RECV, fd);
                pool.append(task);
            }
            else if (events[i].events & EPOLLOUT) // 广播发送到其他客户端
            {
                printf("detect a EPOLLOUT event from %d\n", fd);
                ChatRoomTask* task = new ChatRoomTask(TASK_SEND, fd);
                pool.append(task);
            }
            else
            {
                printf("something else happened\n");
            }
        }
    }

    close(s);
    close(efd);
    return 0;
}
