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
    Client *client;
    string ip, portnum;
    struct addrinfo condtns, *server; // server contains information about the server we are connecting to
    int socketid;                     // for the listening socket

public:
    string name;

    ConnManager(string ip, string portnum, string name);
    ~ConnManager();

    int setupAndConnect();
};