# 高级I/O函数

Linux提供很多高级的I/O函数，在特定条件下能够表现出优秀的性能。
与网络编程相关的函数中，大致可以分为三类：

- 用于创建文件描述符的函数，包括 `pipe`、`dup/dup2`函数
- 用于读写数据的函数，包括 `readv/writev`、`sendfile`、`mmap/munmap`、`splice` 和 `tee` 函数
- 用于控制I/O行为和属性的函数，包括 `fcntl` 函数


