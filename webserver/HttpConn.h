#ifndef __WEBSERVER_HTTPCONNECTION_H
#define __WEBSERVER_HTTPCONNECTION_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#define READ_BUFFER_SIZE 2048
#define WRITE_BUFFER_SIZE 1024
#define FILENAME_LEN 200

/* HTTP 1.1 */
class HttpConn {
public:
    /* HTTP请求方法 */
    enum METHOD {
        GET,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };

    /* 解析客户请求时，主状态机所处的状态 */
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    /* 行的读取状态 */
    enum LINE_STATUS {
        LINE_OK, 
        LINE_BAD, 
        LINE_OPEN
    };

    /* 服务器处理HTTP请求的可能结果 */
    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

public:
    HttpConn() {}
    ~HttpConn() {}

public:
    void init(int sockfd, const sockaddr_in& addr);
    void close_coon(bool real_close = true);
    void process();
    bool read();
    bool write();

private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);

    /* process_read分析HTTP请求 */
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_header(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char* get_line() {return m_read_buf + m_start_line;}
    LINE_STATUS parse_line();

    void unmap();
    bool add_response(const char* format, ...);
    bool add_content(const char* content);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_cnt;

private:
    int m_sockfd;
    
    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx;
    int m_checked_idx;
    int m_start_line;
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;

    /* 主状态机当前状态 */
    CHECK_STATE m_check_state;
    /* 请求方法 */
    METHOD m_method;

    char m_real_file[FILENAME_LEN];
    char* m_url;
    char* m_version;
    char* m_host;
    int m_content_len;
    bool m_linger;

    char* m_file_address;
    
};

#endif
