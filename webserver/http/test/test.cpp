#include "HttpRequest.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <iostream>
#include <cassert>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>

char buffer[2048] = "GET /helloworld.txt HTTP/1.1\r\nHost: developer.mozilla.org\r\nContent-Length: 64\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nname=Joe%20User&request=Send%20me%20one%20of%20your%20catalogue";

int setFdNonBlock(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1) return -1;

    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1) return -1;
    return 0;
}

void print_http(HttpType http) {
    printf("request_line:\n");
    printf("\tmethod:%d\n", http._request_line._method);
    printf("\turl:%s\n", http._request_line._url.c_str());

    printf("code:%d\n", http._code);
    printf("header_num:%d\n", http._headers.size());
    for (int i = 0; i < http._headers.size(); i++) {
        printf("headers[%d]:\n", i);
        printf("\theader_type:%s\n", http._headers[i]._header_type.c_str());
        printf("\tcontent:%s\n", http._headers[i]._content.c_str());
    }
    printf("content:%s\n", http._content);
}

int main(int argc, char *argv[]) {
    char new_buffer[2048]{};

    if (argc <= 2)
    {
        std::cout << "usage: " << argv[0] << " ip_address port_number" << std::endl;
        return -1;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int ret;

    int s = socket(PF_INET, SOCK_STREAM, 0);
    assert(s >= 0);

    sockaddr_in addr;
    bzero(&addr, sizeof(addr)); // 初始化为零
    addr.sin_family = AF_INET;
    inet_aton(ip, &(addr.sin_addr));
    addr.sin_port = htons(port);

    ret = bind(s, (sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind error");
        return -1;
    }

    ret = listen(s, 5);
    if (ret < 0) {
        perror("listen error");
        return -1;
    }

    sockaddr_in addr_client;
    socklen_t addr_client_len = sizeof(addr_client);
    int new_fd = accept(s, (sockaddr *)&addr_client, &addr_client_len);
    setFdNonBlock(new_fd);
    printf("accept a client.\n");

    printf("start process\n");
    HttpRequest request(new_fd);
    request.process();
    printf("process done\n");
}
