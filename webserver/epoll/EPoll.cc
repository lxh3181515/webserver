#include "epoll/EPoll.h"
#include <assert.h>
#include <unistd.h>

EPoll::EPoll() {
    _epollfd = epoll_create1(EPOLL_CLOEXEC);
    assert(_epollfd > 0);

    if (pthread_mutex_init(&_mutex, nullptr) != 0) {
        throw std::exception();
    }
}

EPoll::~EPoll() {
    close(_epollfd);
}

void EPoll::addChannel(std::shared_ptr<Channel> channel) {
    int fd = channel->getFd();
    epoll_event event;
    event.data.fd = fd;
    event.events = channel->getEvents();

    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &event) < 0) {
        perror("epoll_add error");
    }
    pthread_mutex_lock(&_mutex);
    _fd2chan[fd] = channel;
    pthread_mutex_unlock(&_mutex);
}

void EPoll::modChannel(std::shared_ptr<Channel> channel) {
    int fd = channel->getFd();
    int events = channel->getEvents();
    modChannel(fd, events);
}

void EPoll::modChannel(int fd, uint32_t events) {
    epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &event) < 0) {
        perror("epoll_mod error");
        pthread_mutex_lock(&_mutex);
        _fd2chan[fd].reset();
        pthread_mutex_unlock(&_mutex);
    }
}

void EPoll::delChannel(std::shared_ptr<Channel> channel) {
    int fd = channel->getFd();
    delChannel(fd);
}

void EPoll::delChannel(int fd) {
    if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) < 0) {
        perror("epoll_del error");
    }
    pthread_mutex_lock(&_mutex);
    _fd2chan[fd].reset();
    pthread_mutex_unlock(&_mutex);
}

std::vector<std::shared_ptr<Channel>> EPoll::poll() {
    while (true) {
        int event_count = epoll_wait(_epollfd, _events, MAX_EVENTS_NUM, -1);
        if (event_count < 0) perror("epoll wait error");
        std::vector<std::shared_ptr<Channel>> req_data = getEventsRequest(event_count);
        if (req_data.size() > 0) return req_data;
    }
}

std::vector<std::shared_ptr<Channel>> EPoll::getEventsRequest(int events_num) {
    std::vector<std::shared_ptr<Channel>> req_data;
    for (int i = 0; i < events_num; ++i) {
        int fd = _events[i].data.fd;

        std::shared_ptr<Channel> cur_req = _fd2chan[fd];
        cur_req->setREvents(_events[i].events);

        if (cur_req) {
            req_data.push_back(cur_req);
        }
    }
    return req_data;
}
