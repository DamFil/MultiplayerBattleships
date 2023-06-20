#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include "GameState.h"

using namespace std;

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
    Player(int sockfd, GameState *gameinfo) : sockfd(sockfd), gameinfo(gameinfo), status(threadvalue::good), turn(false), ready(false), initialized(false), name("") {}

    ~Player()
    {
        close(sockfd);
    }

    //* gets the name and asks to start the game
    threadvalue getNameAndStart()
    {
        //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GETTING THE NAME OF THE PLAYER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // we only initialize the name if it was not initialized
        if (name == "")
        {
            // getting the header
            bytes_rec = recv(this->sockfd, &header, sizeof(header), 0);
            int name_len = ntohl(this->header); // converting to server byte ordering

            // reading the actual name
            char buf[name_len];
            bytes_rec = recv(this->sockfd, buf, name_len, MSG_WAITALL); // waiting for all the bytes
            threadvalue res = checkBytesRec();
            if (res != threadvalue::good)
                return res;

            this->name = buf;
        }

        // getting the answer for starting the game
        char sg;
        bytes_rec = recv(this->sockfd, &sg, sizeof(char), 0); // sizeof char == 1 byte
        threadvalue res = checkBytesRec();
        if (res != threadvalue::good)
            return res;

        if (sg == 'Y')
        {
            gameinfo->changeStartGame(true);
        }
        else if (sg == 'Q')
        {
            return threadvalue::disconnected;
        }

        string pre_game = "Starting...";
        tmp = pre_game.length();
        header = htonl(tmp);
        bytes_sent = send(this->sockfd, &header, sizeof(int), 0);

        bytes_sent = send(this->sockfd, pre_game.c_str(), header, 0);

        return threadvalue::good;
    }

    void playerInitThread()
    {
        // Intilizing the ships
        cout << "Getting " << this->name << "'s ship positions..." << endl;
        if (!getAllShips())
            return;

        this->initialized = true;
    }

    void closeSocket()
    {
        lock_guard<mutex> l(this->player_mutex);
        close(this->sockfd);
    }

    string getName()
    {
        lock_guard<mutex> l(this->player_mutex);
        return this->name;
    }

private:
    threadvalue checkBytesRec()
    {
        switch (this->bytes_rec)
        {
        case -1:
            return threadvalue::localerr;
            break;
        case 0:
            return threadvalue::disconnected;
            break;
        default:
            return threadvalue::good;
        }
    }

    bool getShip()
    {
        int col, row;

        // aircraft carrier
        this->bytes_rec = recv(this->sockfd, &this->tmp, sizeof(col), 0);
        if (!checkBytesRec())
            return false;
        col = ntohl(tmp);

        this->bytes_rec = recv(this->sockfd, &this->tmp, sizeof(col), 0);
        if (!checkBytesRec())
            return false;
        row = ntohl(tmp);

        pair<int, int> coords(col, row);

        char orientation;
        this->bytes_rec = recv(this->sockfd, &orientation, sizeof(char), 0);
        if (!checkBytesRec())
            return false;

        pair<pair<int, int>, char> ship(coords, orientation);
        this->ship_pos.push_back(ship);

        return true;
    }

    bool getAllShips()
    {
        if (!getShip()) // aircraft carrier
            return false;
        if (!getShip()) // battleship
            return false;
        if (!getShip()) // cruiser
            return false;
        if (!getShip()) // destroyer
            return false;
        if (!getShip()) // destroyer
            return false;
        if (!getShip()) // submarine
            return false;
        if (!getShip()) // submarine
            return false;

        return true;
    };
};