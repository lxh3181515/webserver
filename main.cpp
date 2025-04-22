#include "Server.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        std::cout << "usage: " << argv[0] << " port_number" << std::endl;
        return -1;
    }
    int port = atoi(argv[1]);

    Server server(port);
    server.start();

    return 0;
}
