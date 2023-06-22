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
#include "../Game/NewPlayer.h"

using namespace std;

enum clientvalue
{
    good,
    quit,
    disconnected,
    localerr
};

class Client
{
private:
    clientvalue status;
    int socketid;
    string name;
    NewPlayer *p;

public:
    Client(int socketid, string name);
    ~Client();

    clientvalue initPlayer();

private:
    void sendMessage(string message);
    bool parseCell(string cell, char *col, int *row);
    void initAndSendShip(ShipType t);
    void initSendAllShips();
};