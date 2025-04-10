#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cassert>
#include <string.h>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        cout << "usage: " << argv[0] <<" ip_address port_number" << endl;
        return -1;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);

    int s = socket(PF_INET, SOCK_STREAM, 0);
    assert(s >= 0);

    sockaddr_in addr;
    bzero(&addr, sizeof(addr)); // 初始化为零
    addr.sin_family = AF_INET;
    int ret = inet_aton(ip, &(addr.sin_addr));
    addr.sin_port = htons(port);

    ret = connect(s, (sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        cout << "connect failed." << endl;
    } else {
        // const char* normal_buf = "123\n";
        // const char* oob_buf = "abc";
        // send(s, normal_buf, strlen(normal_buf), 0);
        // send(s, oob_buf, strlen(oob_buf), MSG_OOB);
        // send(s, normal_buf, strlen(normal_buf), 0);
        char buf[256];
        memset(buf, '\0', sizeof(buf));
        recv(s, buf, 256-1, 0);
        cout << buf;
    }

    close(s);
    return 0;
}
