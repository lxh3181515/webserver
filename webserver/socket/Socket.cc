#include "socket/Socket.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

Socket::Socket(int domain, int type) 
: _domain(domain), _type(type) {
    _fd = socket(_domain, _type, 0);
    if (_fd < 0) {
        perror("create socket error");
        assert(0);
    }
}

Socket::~Socket() {
    close(_fd);
    if (!_addr) {
        delete _addr;
    }
}

bool Socket::bindAddr(const char* ip, const int port) {
    switch (_domain)
    {
    case PF_INET:
    {
        sockaddr_in* addr_in = new sockaddr_in;
        bzero(addr_in, sizeof(*addr_in));
        addr_in->sin_family = AF_INET;
        inet_aton(ip, &(addr_in->sin_addr));
        addr_in->sin_port = htons(port);
        _addr = (sockaddr*)addr_in;
        break;
    }
    // case PF_INET6:
    // {
    //     sockaddr_in6* addr_in6 = new sockaddr_in6;
    //     bzero(addr_in6, sizeof(*addr_in6));
    //     addr_in6->sin6_family = AF_INET6;
    //     addr_in6->sin6_flowinfo = 0;
    //     addr_in6->sin6_port = htons(port);
    //     inet_pton(AF_INET6, ip, &(addr_in6->sin6_addr));
    //     _addr = (sockaddr*)&addr_in6;
    //     break;
    // }
    default:
    {
        return false;
        break;
    }
    }

    int ret = bind(_fd, _addr, sizeof(*_addr));
    if (ret < 0) {
        perror("bind error");
        return false;
    }
    _bind = true;
    return true;
}

bool Socket::start(int n) {
    if (!_bind) {
        printf("please bind first\n");
        return false;
    }
    int ret = listen(_fd, n);
    if (ret < 0) {
        perror("listen error");
        return false;
    }
    return true;
}
