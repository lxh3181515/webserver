#ifndef __WEBSERVER_POLLER_H
#define __WEBSERVER_POLLER_H

#include <sys/epoll.h>
#include <Channel.h>

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

private:
    static const int MAX_FD = 100000;

    static const int EVENTS_NUM = 4096;

    int _epollfd;

    std::vector<epoll_event> _events;
    
    std::shared_ptr<Channel> _fd2chan[MAX_FD];
};


#endif // __WEBSERVER_POLLER_H