#pragma once

#include "GameState.h"

class Spectator
{
private:
    GameState *gameinfo;
    threadvalue status;
    bool ships_sent;

public:
    Spectator(GameState *gameinfo);
    ~Spectator();

    threadvalue spectateGame();
    threadvalue sendShips(int num_spectators, int num_players);

private:
    vector<tuple<char, int, char>> getAttempts();
    void sendAttempt(int sockfd, tuple<char, int, char> at);
    void sendAllAttempts(int sockfd, Player *p);
    void sendShipTuples(int sockfd, vector<tuple<char, int, char>> ships);

    inline void checkBytesRec(int bytes_rec);
    inline void sendMessage(int sockfd, string msg);
    inline void sendMessage(int sockfd, char msg);
    inline string recvMessage(int sockfd);
    inline char recvChar(int sockfd);
    inline void sendInt(int sockfd, int x);
    inline int recvInt(int sockfd);
};