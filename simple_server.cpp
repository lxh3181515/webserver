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
#define MAX_BUF_SIZE 1024
#define FD_LIMIT 65535

void setNonBlock(int fd)
{
    int option = fcntl(fd, F_GETFL) | O_NONBLOCK;
    fcntl(fd, F_SETFL, option);
}

void addfd(int efd, int fd, uint32_t events)
{
    epoll_event event;
    event.events = events;
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
            ERROR_CHECK(ret, "Error epoll control");
        }
    }
}

struct client_data
{
    sockaddr_in addr;
    char *send_buf = NULL;
    char recv_buf[MAX_BUF_SIZE];
};

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
    client_data *clients = new client_data[FD_LIMIT];
    int client_sockets[5];
    int client_cnt = 0;

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
                int new_s = accept(fd, (sockaddr *)&addr_client, &addr_client_len);
                if (client_cnt < 5)
                {
                    client_data cli;
                    cli.addr = addr_client;
                    clients[new_s] = cli;
                    client_sockets[client_cnt++] = new_s;
                    addfd(efd, new_s, EPOLLIN | EPOLLRDHUP);
                    printf("a client came\n");
                }
                else
                {
                    const char *reply = "too many clents\n";
                    send(new_s, reply, strlen(reply), 0);
                    close(new_s);
                }
            }
            else if (events[i].events & EPOLLRDHUP) // 客户端关闭连接
            {
                int j;
                for (j = 0; j < client_cnt; j++)
                    if (fd == client_sockets[j])
                        break;
                client_sockets[j] = client_sockets[client_cnt - 1];
                client_cnt--;
                close(fd);
                epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
                printf("a client left\n");
            }
            else if (events[i].events & EPOLLIN) // 接收客户端数据
            {
                memset(clients[fd].recv_buf, '\0', sizeof(clients[fd].recv_buf));
                int bytes_num = recv(fd, clients[fd].recv_buf, sizeof(clients[fd].recv_buf) - 1, 0);
                ERROR_CHECK(bytes_num,
                            "Error recv");
                printf("get %d bytes of client data \"%s\" from %d\n", bytes_num, clients[fd].recv_buf, fd);
                if (bytes_num < 0)
                {
                    if (errno != EAGAIN)
                    {
                        client_cnt--;
                        close(fd);
                        epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
                    }
                }
                else if (bytes_num > 0)
                {
                    for (int j = 0; j < client_cnt; j++)
                    {
                        if (fd == client_sockets[j])
                            continue;
                        clients[client_sockets[j]].send_buf = clients[fd].recv_buf;
                        addfd(efd, client_sockets[j], EPOLLOUT);
                    }
                }
            }
            else if (events[i].events & EPOLLOUT) // 广播发送到其他客户端
            {
                if (!clients[fd].send_buf)
                {
                    continue;
                }
                send(fd, clients[fd].send_buf, strlen(clients[fd].send_buf), 0);
                clients[fd].send_buf = NULL;
                addfd(efd, fd, EPOLLIN | EPOLLRDHUP);
            }
            else
            {
                printf("something else happened\n");
            }
        }
    }

    delete clients;
    close(s);
    close(efd);
    return 0;
}
