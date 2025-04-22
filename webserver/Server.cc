#include "Server.h"

const char* LOCAL_HOST = "127.0.0.1";

const int SOCKET_IP_DOMAIN = PF_INET;
const int SOCKET_TYPE = SOCK_STREAM;
const int SOCKET_CONN_REQUEST_LEN = 5;

const int THREADPOOL_THREADS_NUM = 4;
const int THREADPOOL_MAX_REQUESTS = 10000;

Server::Server(int port)
: _stop(false) {
    _listener = new Socket(SOCKET_IP_DOMAIN, SOCKET_TYPE);
    if (!_listener->bindAddr(LOCAL_HOST, port)) {
        assert(0);
    }
    if (setFdNonBlock(_listener->getFd()) < 0) {
        perror("set socket non block failed");
        assert(0);
    }
    _listener_fd = _listener->getFd();
    _epoll = new EPoll;

    std::shared_ptr<Channel> chan = std::make_shared<Channel>(_listener_fd);
    chan->setEevents(Channel::kConnEvent);
    chan->setConnCallback(std::bind(&Server::handleConnection, this));
    _epoll->addChannel(chan);

    // _threadpool = new ThreadPool(THREADPOOL_THREADS_NUM, THREADPOOL_MAX_REQUESTS);
}

Server::~Server()
{
    _stop = true;
    delete _listener;
    delete _epoll;
    delete _threadpool;
}

void Server::start() {
    std::vector<std::shared_ptr<Channel>> events;
    if (!_listener->start(SOCKET_CONN_REQUEST_LEN)) {
        return;
    }
    printf("Server started.\n");
    while (!_stop) {
        events.clear();
        events = _epoll->poll();
        for (auto& ev : events) {
            ev->handleEvent();
        }
    }
}

void Server::handleConnection() {
    sockaddr_in addr_client;
    socklen_t addr_client_len = sizeof(addr_client);
    int accept_fd = accept(_listener_fd, (sockaddr *)&addr_client, &addr_client_len);
    printf("Accept new socket %d.\n", accept_fd);

    /* 超过最大文件描述符限制数 */
    if (accept_fd >= _epoll->MAX_FD) {
        close(accept_fd);
        return;
    }

    /* 设置非阻塞 */
    if (setFdNonBlock(accept_fd) < 0) {
        close(accept_fd);
        return;
    }

    std::shared_ptr<Channel> chan = std::make_shared<Channel>(accept_fd);
    chan->setEevents(Channel::kReadEvent);
    chan->setReadCallback(NULL);
    _epoll->addChannel(chan);
}
