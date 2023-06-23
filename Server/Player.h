#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <mutex>
#include <tuple>
#include <thread>

using namespace std;

#define MAX_SHIPS 7

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
    string name;         // name of the player
    int tmp, header;     // for receiving integers and size of next message respectively
    int bytes_sent, bytes_rec;
    threadvalue status;
    vector<tuple<char, int, char>> ship_pos{}; // keeps track of the ship positions of the player (including the ship oritentations)
    vector<pair<int, int>> attemps{};          // keeps track of the attempts other players made on destroying the ships
    bool ready;

public:
    Player(int sockfd, GameState *gameinfo);

    ~Player();

    //* gets the name and asks to start the game
    threadvalue getNameAndStart();

    void playerInitThread();

    void closeSocket();

    string getName();

    bool getReady();

private:
    threadvalue checkBytesRec();

    void addShip();

    bool getShip();

    bool getAllShips();
};