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
    if (_revents & (EPOLLRDHUP))
    {
        /* 如有异常，直接关闭客户连接 */
        if (_closeCallback) _closeCallback(_fd);
    }
    if (_revents & (EPOLLERR))
    {
        if (_errorCallback) _errorCallback(_fd);
    }
    if (_revents & EPOLLIN && !(_revents & EPOLLRDHUP))
    {
        if (_readCallback) _readCallback(_fd);
        if (_connCallback) _connCallback(_fd);
    }
    if (_revents & EPOLLOUT)
    {
        if (_writeCallback) _writeCallback(_fd);
    }
}
