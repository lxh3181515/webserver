#ifndef __WEBSERVER_SERVER_H
#define __WEBSERVER_SERVER_H

#include "epoll/EPoll.h"
#include "socket/Socket.h"
#include "threadpool/ThreadPool.h"
#include "http/HttpRequest.h"
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
    void handleConnection(int fd);

    void handleRead(int fd);

    void handleWrite(int fd);

    void handleClose(int fd);

private:
    Socket* _listener;

    EPoll* _epoll;

    ThreadPool* _threadpool;

    int _listener_fd;

    bool _stop;
};

#endif
