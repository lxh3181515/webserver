# 网络编程基础API

## Socket地址API

**主机字节序和网络字节序**

- 大端字节序：高位（23-31bit）存在低地址，低位（0-7bit）存在高地址
- 小端字节序：高位存在高地址，低位存在低地址

PC大多采用小端字节序，因此又被称为主机字节序。

大端字节序又称为网络字节序，它给所有接收数据的主机提供了正确解释收到的格式化数据的保证。

```c++
#include <netinet/in.h>
unsigned long int htonl(unsigned long int hostlong);
unsigned short int htons(unsigned short int hostlong);
unsigned long int ntonl(unsigned long int netlong);
unsigned short int ntons(unsigned short int netlong);
```
长整型函数通常用来转换IP地址，短整型函数用来转换端口。


**通用socket地址**

一般用结构体sockaddr表示socket地址。Linux定义了新的结构体来存储更大的地址，而且内存对齐。
```c++
#include <bits/socket.h>
struct sockaddr_storage {
    sa_family_t sa_family; // 地址族
    unsigned long int __ss_align;
    char __ss_padding[128-sizeof(__ss_align)];
}
```

**专用socket地址**

针对不同协议族有专用的socket地址结构体

|协议族|地址族|描述|结构体|
|---|---|---|---|
|PF_UNIX|AF_UNIX|UNIX本地域协议族|sockaddr_un|
|PF_INET|AF_INET|TCP/IPv4协议族|sockaddr_in|
|PF_INET6|AF_INET6|TCP/IPv6协议族|sockaddr_in6|

**地址表达转换**

点分十进制字符串表示的IPv4地址可读性更好，然而在编程中，二进制地址更方便使用。记录日志时则使用字符串。
```c++
#include <arpa/inet.h>
in_addr_t inet_addr(const char* strptr); // 点分十进制字符串->网络字节序
int inet_aton(const char* cp, struct in_addr* inp); // 与inet_addr相同
char* inet_ntoa(struct in_addr in); // 网络字节序->点分十进制字符串
```

## socket基本使用
socket是一个可读、可写、可控制、可关闭的文件描述符。
```c++
#include <sys/types.h>
#include <sys/socket.h>
```
**创建socket**
```c++
int socket(int domain, int type, int protocol);
```
参数解析
- `domain`：告诉系统使用哪个底层协议族。
- `type`：指定服务类型。
  - `SOCK_STREAM`：流服务（TCP）
  - `SOCK_UGRAM`：数据报服务（UDP）
  - `& SOCK_NONBLOCK`：非阻塞
  - `& SOCK_CLOEXEC`：fork创建子进程时在子进程中关闭该socket
- `protocol`：在前两个参数基础下选择具体协议，通常唯一，0表示默认协议。

**命名socket**
```c++
int bind(int sockfd, const struct sockaddr* my_addr, socklen_t addrlen);
```
将 `my_addr` 所指的socket地址分配给未知名的 `sockfd` 文件描述符，`addrlen` 指出该socket地址的长度。

**监听socket**

```c++
int listen(int sockfd, int backlog);
```
监听队列的长度如果超过 `backlog`，服务器将不再受理新的客户连接，客户端也将收到 `ECONNREFUSED` 错误信息。

**接受连接**
```c++
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
accept成功时返回一个新的连接socket，该socket唯一标识了被接受的这个连接，服务器可以通过读写该socket来与被接受连接对应的客户端通信。

accept只是从监听队列中取出连接，不会考虑连接处于何种状态，更不关心任何网络状况的变化。

**发起连接**
```c++
int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
```
客户端通过调用connect与服务器建立连接。

**关闭连接**
```c++
int close(int fd);
```
close系统调用并非总是立即关闭一个连接，而是将fd的引用计数减1，只有当fd引用计数为0时，才真正关闭连接。

多进程程序中，一次fork系统调用默认使用父进程中打开的socket的引用计数加1，因此必须在父子进程中都对socket执行close调用才能将连接关闭。

socket可以被立即终止连接
```c++
int shutdown(int sockfd, int howto);
```
`howto` 参数决定了shutdown的行为
- `SHUT_RD`：关闭读，接收缓冲区数据丢弃
- `SHUT_WR`：关闭写，发送缓冲区数据在真正关闭前全部发送出去
- `SHUT_RDWR`：同时关闭读和写

