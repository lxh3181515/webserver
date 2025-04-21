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
#include "EPoll.h"

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

    EPoll epoll;
    std::shared_ptr<Channel> chan = std::make_shared<Channel>(s);
    chan->setEevents(Channel::kConnEvent);
    epoll.addChannel(chan);
    ThreadPool pool;

    while (1)
    {
        std::vector<std::shared_ptr<Channel>> channels;

        channels = epoll.poll();

        for (auto iter : channels)
        {
            int fd = iter->getFd();
            int events = iter->getEvents();
            if ((fd == s) && (events & EPOLLIN)) // 连接请求
            {
                sockaddr_in addr_client;
                socklen_t addr_client_len = sizeof(addr_client);
                int new_fd = accept(fd, (sockaddr *)&addr_client, &addr_client_len);
                if (ChatRoomTask::sm_client_num < MAX_CLIENT_NUM)
                {
                    setNonBlock(new_fd);
                    std::shared_ptr<Channel> chan = std::make_shared<Channel>(new_fd);
                    chan->setEevents(Channel::kReadEvent);
                    epoll.addChannel(chan);

                    ChatRoomTask::client_data data;
                    data.addr = addr_client;
                    data.send_buf = nullptr;
                    ChatRoomTask::sm_clients.insert(std::pair<int, ChatRoomTask::client_data>(new_fd, data));
                    ChatRoomTask::sm_client_num++;
                    printf("a client came\n");
                }
                else
                {
                    const char *reply = "too many clents\n";
                    send(new_fd, reply, strlen(reply), 0);
                    close(new_fd);
                }
            }
            else if (events & EPOLLRDHUP) // 客户端关闭连接
            {
                BaseRequest* task = new ChatRoomTask(TASK_CLOSE, fd);
                pool.append(task);
            }
            else if (events & EPOLLIN) // 接收客户端数据
            {
                printf("detect a EPOLLIN event from %d\n", fd);
                BaseRequest* task = new ChatRoomTask(TASK_RECV, fd);
                pool.append(task);
            }
            else if (events & EPOLLOUT) // 广播发送到其他客户端
            {
                printf("detect a EPOLLOUT event from %d\n", fd);
                BaseRequest* task = new ChatRoomTask(TASK_SEND, fd);
                pool.append(task);
            }
            else
            {
                printf("something else happened\n");
            }
        }
    }

    close(s);
    return 0;
}
