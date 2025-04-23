#include "HttpRequest.h"

const char* doc_root = "/home/lingxh";

void HttpRequest::process() {
    HttpType http;
    http._code = HTTP_CODE_NO_REQUEST;
    http._buffer = _read_buffer;

    /* 读取并转换数据 */
    while (read() && http._code == HTTP_CODE_NO_REQUEST) {
        http._buffer_len = _read_len;
        _parser.bufferToHttp(&http);
    }
    if (http._code == HTTP_CODE_NO_REQUEST) {
        return;
    }

    /* 数据解析 */
    HttpType response;
    if (http._code == HTTP_CODE_GET_REQUEST) {
        response = doRequest(&http);
    }

    /* 数据发送 */
    response._buffer = _write_buffer;
    if (_parser.httpToBuffer(&response, MAX_BUF_SIZE)) {
        _write_len = response._buffer_len;
        write();
    }
}

bool HttpRequest::read() {
    if (_read_len >= MAX_BUF_SIZE) {
        printf("Warning: read buffer is too short.\n");
        return false;
    }

    int bytes_num = 0;
    while (true)
    {
        bytes_num = recv(_fd, _read_buffer + _read_len, MAX_BUF_SIZE - _read_len, 0);
        if (bytes_num < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return false;
        }
        else if (bytes_num == 0)
        {
            return false; 
        }
        _read_len += bytes_num;
    }
    return true;
}

bool HttpRequest::write() {
    int bytes_have_send = 0;
    int bytes_to_send = _write_len;
    if (_iv_count == 2) {
        bytes_to_send += _file_stat.st_size;
    }
    if (bytes_to_send == 0) {
        return true;
    }

    _iv[0].iov_base = _write_buffer;
    _iv[0].iov_len = _write_len;

    while (true) {
        int ret = writev(_fd, _iv, _iv_count);
        if (ret < 0) {
            if (errno == EAGAIN) {
                return true;
            }
            unmap();
            return false;
        }

        bytes_have_send += ret;
        if (bytes_have_send >= bytes_to_send) {
            unmap();
            if (_linger) {
                return true;
            }
            return false;
        }
    }
}

void HttpRequest::unmap() {
    if (_file_addr) {
        munmap(_file_addr, _file_stat.st_size);
        _file_addr = 0;
    }
}

HttpType HttpRequest::doRequest(const HttpType* request_http) {
    HttpType res;

    for (const auto & it : request_http->_headers) {
        if (strcasecmp(it._header_type.c_str(), "Connection") != 0) {
            continue;
        }
        if (strcasecmp(it._content.c_str(), "keep-alive") == 0) {
            _linger = true;
        }
        break;
    }

    /* 目前只支持GET方法 */
    do {
        if (request_http->_request_line._method == HTTP_METHOD_GET) {
            char read_file[256] = {};
            int len = strlen(doc_root);
    
            strcpy(read_file, doc_root);
            strncpy(read_file + len, request_http->_request_line._url.c_str(), 256 - len - 1);
    
            if (stat(read_file, &_file_stat) < 0) {
                res._code = HTTP_CODE_NO_RESOURCE;
                break;
            }
            if (!(_file_stat.st_mode & S_IROTH)) {
                res._code = HTTP_CODE_FORBIDDEN_REQUEST;
                break;
            }
            if (S_ISDIR(_file_stat.st_mode)) {
                res._code = HTTP_CODE_BAD_REQUEST;
                break;
            }
            int fd = open(read_file, O_RDONLY);
            _file_addr = (char *)mmap(0, _file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            close(fd);

            _iv[1].iov_base = _file_addr;
            _iv[1].iov_len = _file_stat.st_size;
            _iv_count = 2;
    
            Header h;
            h._header_type = "Content-Length";
            h._content = std::to_string(_file_stat.st_size);
            res._headers.push_back(h);
            h._header_type = "Connection";
            h._content = (_linger == true) ? "keep-alive" : "close";
            res._headers.push_back(h);
            res._code = HTTP_CODE_FILE_REQUEST;
        }
        else {
            res._code = HTTP_CODE_BAD_REQUEST;
        }
    }while (false);
    
    return res;
}
