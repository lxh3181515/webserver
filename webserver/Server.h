#ifndef __WEBSERVER_SERVER_H
#define __WEBSERVER_SERVER_H

#include "epoll/EPoll.h"
#include "socket/Socket.h"
#include "threadpool/ThreadPool.h"
#include <functional>
#include <assert.h>
#include <unistd.h>
#include "Utils.h"

class Server
{
public:
    Server(int port);

    ~Server();

    void start();

private:
    void handleConnection();

private:
    Socket* _listener;

    EPoll* _epoll;

    ThreadPool* _threadpool;

    int _listener_fd;

    bool _stop;
};

#endif
