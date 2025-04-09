#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cassert>
#include <strings.h>
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

    ret = bind(s, (sockaddr *)&addr, sizeof(addr));
    assert(ret == 0);

    ret = listen(s, 5);
    assert(ret == 0);

    sleep(20); // 暂停20秒等待操作
    sockaddr_in addr_client;
    socklen_t addr_client_len = sizeof(addr_client);
    int new_s = accept(s, (sockaddr *)&addr_client, &addr_client_len);
    if (new_s < 0) {
        cout << "accept failed." << endl;
    } else {
        cout << "accept success." << endl;
        cout << inet_ntoa(addr_client.sin_addr) << endl;
    }

    close(s);
    return 0;
}
