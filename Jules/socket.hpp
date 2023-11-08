#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

#define PORT 9999

class TcpServer
{
    private:

    public:
        TcpServer(std::string ip, int port);
        ~TcpServer();
};