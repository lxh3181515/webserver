#include "http/HttpParser.h"

const std::map<std::string, HTTP_METHOD> str2method {
    {"GET", HTTP_METHOD_GET},
    {"POST", HTTP_METHOD_POST},
    {"HEAD", HTTP_METHOD_HEAD},
    {"PUT", HTTP_METHOD_PUT},
    {"DELETE", HTTP_METHOD_DELETE},
    {"TRACE", HTTP_METHOD_TRACE},
    {"OPTIONS", HTTP_METHOD_OPTIONS},
    {"CONNECT", HTTP_METHOD_CONNECT},
    {"PATCH", HTTP_METHOD_PATCH}
};

void HttpParser::reset() {
    m_check_state = CHECK_STATE_REQUESTLINE;

    m_read_buf = nullptr;
    m_read_buf_len = 0;
    m_checked_idx = 0;
    m_start_line = 0;

    m_write_buf = nullptr;
    m_write_buf_len = 0;
    m_write_idx = 0;

    m_http_info = nullptr;
}

char* HttpParser::get_line() {
    return m_read_buf + m_start_line;
}

HttpParser::LINE_STATE HttpParser::parse_line()
{
    char tmp;
    for (; m_checked_idx < m_read_buf_len; ++m_checked_idx)
    {
        tmp = m_read_buf[m_checked_idx];
        if (tmp == '\r')
        {
            if ((m_checked_idx + 1) == m_read_buf_len)
            {
                return LINE_STATE_OPEN;
            }
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_STATE_OK;
            }
            return LINE_STATE_BAD;
        }
        else if (tmp == '\n')
        {
            if (m_checked_idx > 1 && (m_read_buf[m_checked_idx - 1] == '\r'))
            {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_STATE_OK;
            }
            return LINE_STATE_BAD;
        }
    }
    return LINE_STATE_OPEN;
}

HTTP_CODE HttpParser::parse_request_line(char *line)
{
    char* & url = m_http_info->_request_line._url;

    /* 获取请求 */
    url = strpbrk(line, " \t");
    if (!url)
    {
        return HTTP_CODE_BAD_REQUEST;
    }
    *url++ = '\0';

    std::string method(line, (url - 1));
    if (str2method.find(method) != str2method.end())
    {
        m_http_info->_request_line._method = str2method.at(method);
    }
    else
    {
        return HTTP_CODE_BAD_REQUEST;
    }

    /* 获取目标URL */
    char* version;
    url += strspn(url, " \t"); /* 确保第一个字符非空格 */
    version = strpbrk(url, " \t");
    if (!version)
    {
        return HTTP_CODE_BAD_REQUEST;
    }
    *version++ = '\0';
    if (strncasecmp(url, "http://", 7) == 0) /* 跳过域名或IP */
    {
        url += 7;
        url = strchr(url, '/');
    }
    if (!url || url[0] != '/')
    {
        return HTTP_CODE_BAD_REQUEST;
    }

    /* 获取版本号 */
    version += strspn(version, " \t"); /* 确保第一个字符非空格 */
    if (strcasecmp(version, "HTTP/1.1") != 0)
    {
        return HTTP_CODE_BAD_REQUEST;
    }

    m_check_state = CHECK_STATE_HEADER;
    return HTTP_CODE_NO_REQUEST;
}

HTTP_CODE HttpParser::parse_header(char *line)
{
    if (line[0] == '\0') /* 读取空行 */
    {
        if (m_content_len != 0)
        {
            m_check_state = CHECK_STATE_CONTENT;
            return HTTP_CODE_NO_REQUEST;
        }
        return HTTP_CODE_GET_REQUEST;
    }
    else
    {
        char* colon = strpbrk(line, ":");
        if (!colon)
        {
            return HTTP_CODE_BAD_REQUEST;
        }
        *colon = '\0';
        m_http_info->_headers[m_http_info->_header_num]._header_type = line;
        m_http_info->_headers[m_http_info->_header_num]._content = colon + 2;
        m_http_info->_header_num++;
        if (strcasecmp(line, "Content-Length") == 0)
        {
            m_content_len = atol(colon + 2);
        }
    }
    return HTTP_CODE_NO_REQUEST;
}

HTTP_CODE HttpParser::parse_content(char *line)
{
    if (m_read_buf_len >= (m_content_len + m_checked_idx))
    {
        line[m_content_len] = '\0';
        m_http_info->_content = line;
        return HTTP_CODE_GET_REQUEST;
    }
    return HTTP_CODE_NO_REQUEST;
}

HTTP_CODE HttpParser::process_read()
{
    LINE_STATE line_stat = LINE_STATE_OK;
    HTTP_CODE ret = HTTP_CODE_NO_REQUEST;
    char *line = 0;

    while ((m_check_state == CHECK_STATE_CONTENT && line_stat == LINE_STATE_OK) 
        || (line_stat = parse_line()) == LINE_STATE_OK)
    {
        line = get_line();
        m_start_line = m_checked_idx;
        printf("got 1 http line: %s\n", line);

        switch (m_check_state)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            ret = parse_request_line(line);
            if (ret == HTTP_CODE_BAD_REQUEST)
            {
                return HTTP_CODE_BAD_REQUEST;
            }
            break;
        }
        case CHECK_STATE_HEADER:
        {
            ret = parse_header(line);
            if (ret == HTTP_CODE_BAD_REQUEST)
            {
                return HTTP_CODE_BAD_REQUEST;
            }
            else if (ret == HTTP_CODE_GET_REQUEST)
            {
                return HTTP_CODE_GET_REQUEST;
            }
            break;
        }
        case CHECK_STATE_CONTENT:
        {
            ret = parse_content(line);
            if (ret == HTTP_CODE_GET_REQUEST)
            {
                return HTTP_CODE_GET_REQUEST;
            }
            line_stat = LINE_STATE_OPEN;
            break;
        }

        default:
        {
            return HTTP_CODE_INTERNAL_ERROR;
        }
        }
    }
    return HTTP_CODE_NO_REQUEST;
}

bool HttpParser::bufferToHttp(char* buffer, int buf_len, HttpType* http_type) {
    if (!http_type || !buffer || buf_len <= 0) {
        return false;
    }
    m_read_buf = buffer;
    m_read_buf_len = buf_len;
    m_http_info = http_type;

    bool buffer_good = true;
    m_http_info->_code = process_read();
    if (m_http_info->_code != HTTP_CODE_GET_REQUEST) {
        buffer_good = false;
    }
    reset();
    return buffer_good;
}

/* 

HTTP转换Buffer部分 

*/

bool HttpParser::add_response(const char* format, ...) {
    if (m_write_idx >= m_write_buf_len)
    {
        return false;
    }
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_idx, m_write_buf_len - 1 - m_write_idx, format, arg_list);
    if (len >= (m_write_buf_len - 1 - m_write_idx)) {
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);
    return true;
}

bool HttpParser::add_status_line(int status, const char* title) {
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool HttpParser::add_headers() {
    for (int i = 0; i < m_http_info->_header_num; i++) {
        if (!add_response("%s: ", m_http_info->_headers[i]._header_type))
            return false;
        if (!add_response("%s\r\n", m_http_info->_headers[i]._content))
            return false;
    }
    return add_response("%s", "\r\n");
}

bool HttpParser::add_content() {
    return add_response("%s", m_http_info->_content);
}

bool HttpParser::process_write(HTTP_CODE ret) {
    switch (ret)
    {
    case HTTP_CODE_INTERNAL_ERROR:
    {
        add_status_line(500, ERROR_500_TITLE);
        add_headers();
        if (!add_content()) {
            return false;
        }
        break;
    }
    case HTTP_CODE_BAD_REQUEST:
    {
        add_status_line(400, ERROR_400_TITLE);
        add_headers();
        if (!add_content()) {
            return false;
        }
        break;
    }
    case HTTP_CODE_NO_RESOURCE:
    {
        add_status_line(404, ERROR_404_TITLE);
        add_headers();
        if (!add_content()) {
            return false;
        }
        break;
    }
    case HTTP_CODE_FORBIDDEN_REQUEST:
    {
        add_status_line(403, ERROR_403_TITLE);
        add_headers();
        if (!add_content()) {
            return false;
        }
        break;
    }
    case HTTP_CODE_FILE_REQUEST:
    {
        add_status_line(200, OK_200_TITLE);
        add_headers();
        if (!add_content()) {
            return false;
        }
        break;
    }
    case HTTP_CODE_GET_REQUEST:
    {
        add_status_line(200, OK_200_TITLE);
        add_headers();
        if (!add_content()) {
            return false;
        }
        break;
    }
    default:
    {
        return false;
    }
    }
    return true;
}

bool HttpParser::httpToBuffer(char* buffer, int max_buf_len, HttpType* http_type) {
    m_write_buf = buffer;
    m_write_buf_len = max_buf_len;
    m_http_info = http_type;
    bool ret = process_write(m_http_info->_code);
    reset();
    return ret;
}
