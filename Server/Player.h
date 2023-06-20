#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <mutex>

using namespace std;

class GameState;

enum threadvalue
{
    disconnected,
    localerr,
    good
};

class Player
{
private:
    mutex player_mutex;
    int sockfd;          // the socket of the player connecting it to the client
    GameState *gameinfo; // keeps track of the state of the game
    int tmp;
    int header;                // specifies the size of the message about to be received - as defined by the protocol used
    int bytes_sent, bytes_rec; // object wide vars for keeping track of the #bytes sent and rec
    threadvalue status;
    bool turn, ready;                              // turn - true if it is this players turn right now, ready - true if the client replied with Y
    bool initialized;                              // true - if the players name, ready status and ships positions have all been inits
    string name;                                   // name of the player
    vector<pair<pair<int, int>, char>> ship_pos{}; // keeps track of the ship positions of the player (including the ship oritentations)
    vector<pair<int, int>> attemps{};              // keeps track of the attempts other players made on destroying the ships

public:
    Player(int sockfd, GameState *gameinfo);

    ~Player();

    //* gets the name and asks to start the game
    threadvalue getNameAndStart();

    void playerInitThread();

    void closeSocket();

    string getName();

private:
    threadvalue checkBytesRec();

    bool getShip();

    bool getAllShips();
};