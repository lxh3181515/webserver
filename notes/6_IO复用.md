# I/O复用

Linux下实现I/O复用的系统调用主要有select、poll和epoll。

## select系统调用

在一段指定时间内，监听用户感兴趣的文件描述符上的可读、可写和异常等事件。

**API**

```c++
#include <sys/select.h>
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
```
参数解释
- `nfds`：指定被监听的文件描述符总数。
- `readfds`、`writefds`、`exceptfds`：可读可写和异常等事件对应的文件描述符。select调用返回时，内核将修改它们来通知应用程序哪些文件描述符已经就绪。
- `timeout`：设置select函数的超时时间。如果传0，立即返回；如果传递NULL，则阻塞至某个文件描述符就绪。
- 返回值：文件描述符总数，失败时返回-1并设置errno。

**文件描述符就绪条件**

网络编程中，下列情况socket可读：
- 内核接收缓存区中的字节数大于等于其低水位标记
- socket通信的对方关闭连接
- 监听socket上有新的连接请求
- socket上有未处理的错误

下列情况下socket可写：
- socket内核发送缓存区可用字节数大于等于低水位标记
- socket的写操作被关闭。此时执行写操作触发SIGPIPE信号
- socket使用非阻塞connect连接成功或者失败后
- socket上有未处理的错误

异常情况只有一种：socket上接收到带外数据

## poll系统调用

与select类似，在指定时间内轮询一定数量的文件描述符。
```c++
#include <poll.h>
int poll(struct pollfd* fds, nfds_t nfds, int timeout);
```
参数解释
- `fds`：指定感兴趣的文件描述符上发生的可读可写和异常等事件。结构体包含
  - `fd`：文件描述符
  - `events`：注册的事件(监听`fd`上的哪些事件)
  - `revents`：实际发生的事件，由内核填充
- `nfds`：指定被监听事件集合`fds`的大小
- `timeout`：指定poll的超时值

## epoll系列系统调用

epoll是linux特有的I/O复用函数，它在实现上与select、poll有很大差异。

**内核事件表**

epoll把用户关心的文件描述符上的事件放在内核里的一个事件表中，每次调用无需重复传入文件描述符或事件集合。但epoll需要使用一个额外的文件描述符。

```c++
#include <sys/epoll.h>
int epoll_create(int size); // size提示内核事件表需要多大
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```
参数解释
- `op`：指定操作类型，有以下3种：
  - `EPOLL_CTL_ADD`，往事件表中注册fd上的事件
  - `EPOLL_CTL_MOD`，修改fd上的注册事件
  - `EPOLL_CTL_DEL`，删除fd上的注册事件
- `event`：结构体成员包含
  - `events`：epoll事件。epoll支持的事件类型与poll基本相同。但epoll有两个额外事件类型——`EPOLLET`和`EPOLLONESHOT`，对于epoll的高效运作非常关键。
  - `data`：用户数据。一般是一个文件描述符。

**epoll_wait函数**

在一段时间内等待一组文件描述符上的事件
```c++
int epoll_wait(int epfd, struct epoll_event *events, 
               int maxevents, int timeout);
```

如果检测到事件，则将所有就绪的事件从内核事件表中复制到它的第二个参数events指向的数组中（只用于输出）。

**LT和ET模式**

epoll对文件描述符的操作有两种模式：电平触发（LT）和边沿触发（ET）模式。默认为LT模式。往内核事件表注册一个文件描述符上的`EPOLLET`事件来切换到ET模式。

- LT模式：epoll_wait检测到事件并通知应用程序后，应用程序可以不立即处理，后续调用epoll_wait还会再次通知；
- ET模式：未处理的事件不会重复通知。很大程度降低了同一个epoll事件被重复触发的次数，效率比LT模式高。

**EPOLLONESHOT事件**


