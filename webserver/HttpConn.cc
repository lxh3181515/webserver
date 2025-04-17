#include "HttpConn.h"

#define OK_200_TITLE "OK"
#define ERROR_400_TITLE "Bad Request"
#define ERROR_400_FORM "Your request has bad syntax or is inherently impossible to satisfy.\n"
#define ERROR_403_TITLE "Forbidden"
#define ERROR_403_FORM "You do not have permission to get file from this server.\n"
#define ERROR_404_TITLE "Not Found"
#define ERROR_404_FORM "The requested file was not found on this server.\n"
#define ERROR_500_TITLE "Internal Error"
#define ERROR_500_FORM "There was an unsual problem serving the requested file.\n"

HttpConn::LINE_STATUS HttpConn::parse_line() {
    char tmp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx) {
        tmp = m_read_buf[m_checked_idx];
        if (tmp == '\r') {
            if ((m_checked_idx + 1) == m_read_idx) {
                return LINE_OPEN;
            }
            else if (m_read_buf[m_checked_idx + 1] == '\n') {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (tmp == '\n') {
            if (m_checked_idx > 1 && (m_read_buf[m_checked_idx - 1] == '\r')) {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

HttpConn::HTTP_CODE HttpConn::process_read() {
    LINE_STATUS line_stat = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char* text = 0;

    while ((m_check_state == CHECK_STATE_CONTENT && line_stat == LINE_OK)
        || (line_stat = parse_line()) == LINE_OK)
    {
        text = get_line();
        m_start_line = m_checked_idx;
        printf("got 1 http line: %s\n", text);

        switch (m_check_state)
        {
            case CHECK_STATE_REQUESTLINE:
            {
                ret = parse_request_line(text);
                if (ret == BAD_REQUEST) {
                    return BAD_REQUEST;
                }
                break;
            }
            case CHECK_STATE_HEADER:
            {
                ret = parse_header(text);
                if (ret == BAD_REQUEST)
                {
                    return BAD_REQUEST;
                }
                else if (ret == GET_REQUEST)
                {
                    return do_request();
                }
                break;
            }
            case CHECK_STATE_CONTENT:
            {
                ret = parse_content(text);
                if (ret == GET_REQUEST)
                {
                    return do_request();
                }
                line_stat = LINE_OPEN;
                break;
            }
        
            default:
            {
                return INTERNAL_ERROR;
            }
        }
    }
    return NO_REQUEST;
}

void HttpConn::process() {
    HTTP_CODE read_ret = process_read();
}
