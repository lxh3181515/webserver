#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cassert>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cerrno>

using namespace std;


void recv_once(int socket, char *buf, size_t buf_len, int flag=0) {
    memset(buf, '\0', buf_len);
    int ret;
    if (sockatmark(socket)) {
        cout << "oob ";
        ret = recv(socket, buf, buf_len-1, MSG_OOB);
    } else {
        ret = recv(socket, buf, buf_len-1, flag);
    }
    if (ret < 0) {
        perror("Error recv");
        return;
    }
    cout << "recv:" << buf << endl;
}


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

    ret = bind(s, (sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("Error binding");
        return -1;
    }

    ret = listen(s, 5);
    assert(ret == 0);

    sockaddr_in addr_client;
    socklen_t addr_client_len = sizeof(addr_client);
    int new_s = accept(s, (sockaddr *)&addr_client, &addr_client_len);
    if (new_s < 0) {
        cout << "accept failed." << endl;
    } else {
        // cout << "connected with ip:" << inet_ntoa(addr_client.sin_addr) << " and port:" << addr_client.sin_port << endl;
        // char buf[256];

        // recv_once(new_s, buf, sizeof(buf));
        // recv_once(new_s, buf, sizeof(buf));
        // recv_once(new_s, buf, sizeof(buf));

        close(STDOUT_FILENO);
        dup(new_s);
        cout << "abcd\n";
        close(new_s);
    }

    close(s);
    return 0;
}
