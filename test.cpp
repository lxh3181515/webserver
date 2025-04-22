#include "HttpParser.h"
#include <stdio.h>

char buffer[2048] = "DELETE /contact_form.php HTTP/1.1\r\nHost: developer.mozilla.org\r\nContent-Length: 64\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nname=Joe%20User&request=Send%20me%20one%20of%20your%20catalogue";

void print_http(HttpType http) {
    printf("request_line:\n");
    printf("\tmethod:%d\n", http._request_line._method);
    printf("\turl:%s\n", http._request_line._url);

    printf("code:%d\n", http._code);
    printf("header_num:%d\n", http._header_num);
    for (int i = 0; i < http._header_num; i++) {
        printf("headers[%d]:\n", i);
        printf("\theader_type:%s\n", http._headers[i]._header_type);
        printf("\tcontent:%s\n", http._headers[i]._content);
    }
    printf("content:%s\n", http._content);
}

int main() {
    HttpParser parser;
    char new_buffer[2048]{};

    HttpType* http = new HttpType;
    bool ret = parser.bufferToHttp(buffer, strlen(buffer)+1, http);
    print_http(*http);
    ret = parser.httpToBuffer(new_buffer, 2048, http);
    if (!ret) {
        printf("httpToBuffer failed\n");
    }else {
        printf("%s\n", new_buffer);
    }
}
