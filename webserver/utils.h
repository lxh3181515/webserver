#ifndef __WEBSERVER_UTILS_H
#define __WEBSERVER_UTILS_H

#include <fcntl.h>

void fdSetNonBlock(int fd)
{
    int option = fcntl(fd, F_GETFL) | O_NONBLOCK;
    fcntl(fd, F_SETFL, option);
}

#endif
