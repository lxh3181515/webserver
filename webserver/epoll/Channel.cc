#include "epoll/Channel.h"
#include <sstream>
#include <assert.h>
#include <sys/epoll.h>


Channel::Channel(int fd)
  : _fd(fd),
    _events(0)
{
}

Channel::~Channel()
{
}

void Channel::handleEvent()
{
    if ((_events & EPOLLHUP) && !(_events & EPOLLIN))
    {
        if (_closeCallback) _closeCallback();
    }
    if (_events & (EPOLLERR))
    {
        if (_errorCallback) _errorCallback();
    }
    if (_events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if (_readCallback) _readCallback();
    }
    if (_events & EPOLLOUT)
    {
        if (_writeCallback) _writeCallback();
    }
    if (_events & EPOLLIN)
    {
        if (_connCallback) _connCallback();
    }
}
