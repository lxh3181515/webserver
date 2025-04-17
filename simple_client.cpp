#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cassert>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cerrno>

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

using namespace std;

void setNonBlock(int fd)
{
    int option = fcntl(fd, F_GETFL) | O_NONBLOCK;
    fcntl(fd, F_SETFL, option);
}

void addfd(int efd, int fd, uint32_t events, bool oneshot=false)
{
    epoll_event event;
    if (oneshot)
        event.events = events | EPOLLONESHOT;
    else 
        event.events = events;
    event.data.fd = fd;
    ERROR_CHECK(epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event),
                "Error epoll control");
    setNonBlock(fd);
}

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
    int ret = inet_aton(ip, &(addr.sin_addr));
    addr.sin_port = htons(port);

    int efd = epoll_create(5);

    ret = connect(s, (sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        close(s);
        perror("Error connet");
        return -1;
    }

    addfd(efd, 0, EPOLLIN);              // 标准输入
    addfd(efd, s, EPOLLIN | EPOLLRDHUP); // socket可读事件

    int pipefd[2];
    pipe(pipefd);

    char buf[1024];
    bool flag = true;
    while (flag)
    {
        epoll_event events[MAX_EVENT_NUM];
        int num = epoll_wait(efd, events, MAX_EVENT_NUM, -1);
        for (int i = 0; i < num; i++)
        {
            int fd = events[i].data.fd;
            if (events[i].events & EPOLLIN)
            {
                if (fd == 0)
                {
                    // 零拷贝
                    splice(fd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
                    splice(pipefd[0], NULL, s, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
                }
                else
                {
                    memset(buf, '\0', sizeof(buf));
                    recv(fd, buf, sizeof(buf) - 1, 0);
                    printf("%s", buf);
                }
            }
            if (events[i].events & EPOLLRDHUP)
            {
                printf("server close the connection\n");
                flag = false;
                break;
            }
        }
    }

    close(s);
    return 0;
}
