#pragma once

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include "Client.h"

using namespace std;

class ConnManager
{
private:
    string ip, portnum;
    struct addrinfo condtns, *server; // server contains information about the server we are connecting to
    int socketid;                     // for the listening socket

public:
    ConnManager(string ip, string portnum);
    ~ConnManager();

    int setupAndConnect();
    int getSocket();
    void closeSocket();
};