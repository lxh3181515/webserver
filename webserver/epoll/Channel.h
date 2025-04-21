#ifndef __WEBSERVER_CHANNEL_H
#define __WEBSERVER_CHANNEL_H

#include <functional>
#include <memory>
#include <string>

class Channel
{
public:
    typedef std::function<void()> EventCallback;

    Channel(int fd);
    
    ~Channel();

    void handleEvent();

    void setConnCallback(EventCallback cb)
    { _connCallback = std::move(cb); }

    void setReadCallback(EventCallback cb)
    { _readCallback = std::move(cb); }

    void setWriteCallback(EventCallback cb)
    { _writeCallback = std::move(cb); }

    void setCloseCallback(EventCallback cb)
    { _closeCallback = std::move(cb); }

    void setErrorCallback(EventCallback cb)
    { _errorCallback = std::move(cb); }

    int getFd() const { return _fd; }

    int getEvents() const { return _events; }

    void setEevents(int evt) { _events = evt; }

public:
    static const int kNoneEvent = 0;

    static const int kReadEvent = EPOLLIN | EPOLLPRI;

    static const int kWriteEvent = EPOLLOUT;

    static const int kConnEvent = EPOLLIN;

private:
    const int  _fd;

    int        _events;

    EventCallback _connCallback;

    EventCallback _readCallback;

    EventCallback _writeCallback;

    EventCallback _closeCallback;

    EventCallback _errorCallback;
};

#endif  // __WEBSERVER_CHANNEL_H
