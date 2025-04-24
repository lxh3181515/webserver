#ifndef __WEBSERVER_HTTPREQUEST_H
#define __WEBSERVER_HTTPREQUEST_H

#include "BaseRequest.h"
#include "HttpParser.h"
#include "EPoll.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>

#define MAX_BUF_SIZE 4096

class HttpRequest : public BaseRequest
{
public:
    HttpRequest(int fd, EPoll* epoll) : _fd(fd), _channel_handler(epoll) {}

    ~HttpRequest() {}

    void process() override;

private:
    bool read();

    bool write();

    void unmap();

    void closeConn();

    void resetConn();

    HttpType doRequest(const HttpType* http);

private:
    HttpParser _parser;

    char _read_buffer[MAX_BUF_SIZE] = {};

    char _write_buffer[MAX_BUF_SIZE] = {};

    int _read_len = 0;

    int _write_len = 0;

    bool _linger = false;

    int _fd = -1;

    iovec _iv[2];

    int _iv_count = 1;

    struct stat _file_stat;

    char* _file_addr = nullptr;

    EPoll* _channel_handler;
};

#endif
