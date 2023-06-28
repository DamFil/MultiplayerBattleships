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

#define ERROR_INT -5667
#define ERROR_CHAR '@'

enum clientvalue
{
    good,
    lose,
    quit,
    disconnected,
    localerr
};

class Client
{
private:
    int sockfd;
    string name;
    clientvalue status;
    NewPlayer *p;

public:
    Client(int socketid, string name);
    ~Client();

    clientvalue initPlayer();

    clientvalue initAllShips();
    void sendShip(char col, int row, char orientation);
    bool addShipLocal(ShipType t, char col, int row, char orientation);

    clientvalue attack();

private:
    clientvalue spectate();

    bool parseCell(string cell, char *col, int *row);
    void initShip(ShipType t, char *col, int *row, char *orientation);

    void recvAttempt(char *col, int *row, char *hm);
    vector<tuple<char, int, char>> recvAllAttempts();
    void recvShip(char *co, int *row, char *hm);
    vector<tuple<char, int, char>> recvAllShips();

    void choosePlayerToAttack(vector<string> names);
    void sendStrike();

    inline void checkBytesRec(int bytes_rec);
    inline void sendMessage(string msg);
    inline void sendMessage(char msg);
    inline string recvMessage();
    inline char recvChar();
    inline void sendInt(int x);
    inline int recvInt();
};