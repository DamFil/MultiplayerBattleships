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
#include <thread>
#include <future>
#include "GameState.h"
#include "Spectator.h"
using namespace std;

#define MAX_PLAYERS 4
#define MAX_SPECTATORS 10
#define MAX_BACKLOG_SIZE 10

enum Output
{
    success,
    failure
};

class ConnManager
{
private:
    struct addrinfo condtns;
    struct sockaddr_storage newconn;
    int socketid; // for the listening socket
    string portnum;
    threadvalue status;
    vector<thread> waiting_for_dc{};
    vector<future<threadvalue>> futures{};
    GameState *gameinfo;
    Spectator *spectator;

public:
    ConnManager(string portnum);
    ~ConnManager();

    Output setupAndListen();
    Output acceptConnections();
    void waitForDisconnect(future<threadvalue> fu, Player *p);
    void turnRegulator();
};