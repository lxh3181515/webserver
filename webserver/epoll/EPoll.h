#ifndef __WEBSERVER_POLLER_H
#define __WEBSERVER_POLLER_H

#include <sys/epoll.h>
#include "epoll/Channel.h"
#include <vector>

class EPoll
{

public:
    EPoll();

    ~EPoll();

    void addChannel(std::shared_ptr<Channel> channel);

    void modChannel(std::shared_ptr<Channel> channel);

    void delChannel(std::shared_ptr<Channel> channel);

    std::vector<std::shared_ptr<Channel>> poll();

    std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);

public:
    static const int MAX_FD = 100000;

    static const int MAX_EVENTS_NUM = 4096;

private:
    int _epollfd;

    epoll_event _events[MAX_EVENTS_NUM];
    
    std::shared_ptr<Channel> _fd2chan[MAX_FD];
};


#endif // __WEBSERVER_POLLER_H