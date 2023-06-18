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
    int tmp;
    int header;                       // specifies the size of the message about to be received - as defined by the protocol used
    int bytes_sent, bytes_rec;        // object wide vars for keeping track of the #bytes sent and rec
    string message_sent, message_rec; // object wide vars for keeping track of the messages sent and rec

public:
    int sockfd;                                    // the socket of the player connecting it to the client
    bool turn, ready;                              // turn - true if it is this players turn right now, ready - true if the client replied with Y
    bool initialized;                              // true - if the players name, ready status and ships positions have all been inits
    string name;                                   // name of the player
    vector<pair<pair<int, int>, char>> ship_pos{}; // keeps track of the ship positions of the player (including the ship oritentations)
    vector<pair<int, int>> attemps{};              // keeps track of the attempts other players made on destroying the ships

    Player(int sockfd) : sockfd(sockfd), turn(false), initialized(false) {}
    ~Player()
    {
        close(sockfd);
    }

    void playerInitThread()
    {
        if (!initialized)
        {
            // first expecting a players name
            this->bytes_rec = recv(this->sockfd, &header, sizeof(header), 0);

            int name_len = ntohl(this->header); // converting to server byte ordering

            // reading the actual name
            char buf[name_len];
            this->bytes_rec = recv(this->sockfd, buf, name_len, MSG_WAITALL);
            if (!checkBytesRec())
                return;

            // loading the name into the objet variables
            this->message_rec = buf;
            this->name = buf;

            // receiving confirmation for starting the game
            char sg;
            bytes_rec = recv(this->sockfd, &sg, sizeof(char), 0); // sizeof char == 1 byte
            if (!checkBytesRec())
                return;

            if (sg == 'Y')
                this->ready = true;
            else
            {
                cout << "Player: [ " << this->name << " ] kicked out for not wanting to play the game..." << endl;
                close(sockfd);
                m.lock();
                --num_players;
                m.unlock();
                return;
            }

            // Intilizing the ships
            cout << "Getting " << this->name << "'s ship positions..." << endl;
            if (!getAllShips())
                return;

            this->initialized = true;
        }

        // this is where we start the attacking turns
    }

private:
    bool checkBytesRec()
    {
        if (this->bytes_rec < 0)
        {
            cerr << "Problem with receiving a message from the player: [ " << this->name << " ].\nERRNO: [ " << errno << " ]" << endl;
            close(sockfd);
            m.lock();
            --num_players;
            m.unlock();
            return false;
        }
        else if (this->bytes_rec == 0)
        {
            cout << "Player: [ " << this->name << " ] disconnected..." << endl;
            close(this->sockfd);
            m.lock();
            --num_players;
            m.unlock();
            return false;
        }
    }

    bool checkBytesSent()
    {
        if (this->bytes_sent < 0)
        {
            cerr << "Problem with sending a message to the player: [ " << this->name << " ].\nERRNO: [ " << errno << " ]" << endl;
            close(sockfd);
            m.lock();
            --num_players;
            m.unlock();
            return false;
        }

        return true;
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
    }
};