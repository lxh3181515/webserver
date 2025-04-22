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

using namespace std;

#define ERROR_CHECK(x, str) \
    do                      \
    {                       \
        if (x < 0)          \
        {                   \
            perror(str);    \
            assert(0);      \
        }                   \
    } while (0)

#define MAX_EVENT_NUM 1024

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        cout << "usage: " << argv[0] << " ip_address port_number" << endl;
        return -1;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]);

    int s = socket(PF_INET, SOCK_STREAM, 0);
    assert(s >= 0);

    sockaddr_in addr;
    bzero(&addr, sizeof(addr)); // 初始化为零
    addr.sin_family = AF_INET;
    inet_aton(ip, &(addr.sin_addr));
    addr.sin_port = htons(port);

    ERROR_CHECK(bind(s, (sockaddr *)&addr, sizeof(addr)),
                "Error binding");

    close(s);
    return 0;
}