#ifndef __WEBSERVER_CHATROOMTASK_H
#define __WEBSERVER_CHATROOMTASK_H

#include "BaseTask.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <map>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <semaphore.h>
#include <queue>
#include <math.h>

#define MAX_BUF_SIZE 1024
#define MAX_CLIENT_NUM 3

#define TASK_CLOSE  1 /* 客户端关闭请求 */
#define TASK_RECV   2 /* 接收数据 */
#define TASK_SEND   3 /* 发送数据 */

class ChatRoomTask : BaseTask
{
public:
    struct client_data
    {
        sockaddr_in addr;
        char* send_buf;
        char recv_buf[MAX_BUF_SIZE];
    };

public:
    ChatRoomTask(int task, int fd) : m_task(task), m_fd(fd){};

    ~ChatRoomTask();

    void process() override;

    static void setup(int listen, int epfd);

public:
    static int sm_client_num;

    static std::map<int, client_data> sm_clients;

private:
    int m_task;

    int m_fd;

    static pthread_mutex_t sm_mutex; /* 互斥锁 */

    static int sm_listen;

    static int sm_epfd;
};

void setNonBlock(int fd);
void addfd(int efd, int fd, uint32_t events, bool oneshot=false);

#endif
