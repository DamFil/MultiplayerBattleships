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
#include <ios>
#include <limits>
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

    bool addShipLocal(ShipType t, char col, int row, char orientation);
    clientvalue initPlayer();
    clientvalue sendShip(char col, int row, char orientation);
    clientvalue initAllShips();
    clientvalue attack();

private:
    void sendMessage(string message);
    bool parseCell(string cell, char *col, int *row);
    void initShip(ShipType t, char *col, int *row, char *orientation);
    void recvAttempt(char *col, int *row, char *hm);
    vector<tuple<char, int, char>> recvAllAttempts();
    void choosePlayerToAttack(string names);
    void sendStrike();
    bool hasSpace(string word);
    void displayForeignAttempts(vector<tuple<char, int, char>>);
};