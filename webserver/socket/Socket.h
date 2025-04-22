#ifndef __WEBSERVER_SOCKET_H
#define __WEBSERVER_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

class Socket
{
public:
    Socket(int domain, int type);

    ~Socket();

    bool bindAddr(const char* ip, const int port);

    bool start(int n);

    int getFd() const {return _fd;}

private:
    int _fd;

    int _domain;

    int _type;

    bool _bind = false;

    sockaddr* _addr = nullptr;
};


#endif // __WEBSERVER_SOCKET_H
