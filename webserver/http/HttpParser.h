#ifndef __WEBSERVER_HTTPCONNECTION_H
#define __WEBSERVER_HTTPCONNECTION_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <map>
#include <string>
#include "http/HttpType.h"

/* HTTP 1.1 */
class HttpParser {
protected:
    /* 解析客户请求时，主状态机所处的状态 */
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    /* 行的读取状态 */
    enum LINE_STATE {
        LINE_STATE_OK, 
        LINE_STATE_BAD, 
        LINE_STATE_OPEN
    };

public:
    HttpParser() {}
    ~HttpParser() {}

    bool bufferToHttp(char* buffer, int buf_len, HttpType* http_type);
    bool httpToBuffer(char* buffer, int max_buf_len, HttpType* http_type);

private:
    void reset();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);

    /* process_read */
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_header(char* text);
    HTTP_CODE parse_content(char* text);
    char* get_line();
    LINE_STATE parse_line();

    /* process_write */
    bool add_response(const char* format, ...);
    bool add_status_line(int status, const char* title);
    bool add_content();
    bool add_headers();

private:
    /* 主状态机当前状态 */
    CHECK_STATE m_check_state = CHECK_STATE_REQUESTLINE;

    char* m_read_buf = nullptr;
    int m_read_buf_len = 0;
    int m_checked_idx = 0;
    int m_start_line = 0;
    int m_content_len = 0;

    char* m_write_buf = nullptr;
    int m_write_buf_len = 0;
    int m_write_idx = 0;

    HttpType* m_http_info = nullptr;
};

#endif
