#ifndef __WEBSERVER_HTTPTYPE_H
#define __WEBSERVER_HTTPTYPE_H

#include <vector>
#include <string>

#define OK_200_TITLE "OK"
#define ERROR_400_TITLE "Bad Request"
#define ERROR_403_TITLE "Forbidden"
#define ERROR_404_TITLE "Not Found"
#define ERROR_500_TITLE "Internal Error"

#define ERROR_400_FORM "Your reguest has bad syntax or is inherently impossible to satisfy.\n"
#define ERROR_403_FORM "You do not have permission to get fle from this server.\n"
#define ERROR_404_FORM "The requested file was not found on this server.\n"
#define ERROR_500_FORM "There was an unusual problem serving the requested file.\n"

/* HTTP请求方法 */
enum HTTP_METHOD
{
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_PATCH
};

/* 服务器处理HTTP请求的可能结果 */
enum HTTP_CODE
{
    HTTP_CODE_NO_REQUEST,
    HTTP_CODE_GET_REQUEST,
    HTTP_CODE_BAD_REQUEST,
    HTTP_CODE_NO_RESOURCE,
    HTTP_CODE_FORBIDDEN_REQUEST,
    HTTP_CODE_FILE_REQUEST,
    HTTP_CODE_INTERNAL_ERROR,
    HTTP_CODE_CLOSED_CONNECTION
};

struct RequestLine
{
    HTTP_METHOD _method;
    std::string _url;
};

struct Header
{
    std::string _header_type;
    std::string _content;
};

struct HttpType
{
    char* _buffer = nullptr;
    int _buffer_len = 0;

    RequestLine _request_line;
    std::vector<Header> _headers;
    int _content_len = 0;
    char* _content = nullptr;

    HTTP_CODE _code = HTTP_CODE_NO_REQUEST;
};

#endif
