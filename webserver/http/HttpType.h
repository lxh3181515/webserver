#define OK_200_TITLE "OK"
#define ERROR_400_TITLE "Bad Request"
#define ERROR_403_TITLE "Forbidden"
#define ERROR_404_TITLE "Not Found"
#define ERROR_500_TITLE "Internal Error"

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
    char* _url = nullptr;
};

struct Header
{
    char* _header_type;
    char* _content = nullptr;
};

struct HttpType
{
    HTTP_CODE _code;
    RequestLine _request_line;
    int _header_num = 0;
    Header _headers[30];
    char* _content = nullptr;
};
