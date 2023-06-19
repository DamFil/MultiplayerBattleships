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
#include "GameState.h"
#include "Player.h"

using namespace std;

/*
 *Protocol for sending and receiving packets:
 *The sender always first sends the byte size of the message it is sending
 *and then it sends the actual message
 */

#define MAX_PLAYERS 4
#define MAX_SPECTATORS 10
#define MAX_BACKLOG_SIZE 10

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MAIN~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char *argv[])
{
    // we get the portnum from the command line arguments
    string portnum;

    if (argc != 2)
    {
        cout << "Usage: ./server portnum" << endl;
        portnum = "5001";
    }
    else
        portnum = argv[1];

    vector<thread> player_thread_pool{};    // keeps track of all the active player threads
    vector<thread> spectator_thread_pool{}; // keeps track of all the active spectator threads
    vector<Player *> active_players{};      // keeps track of states of all the currently active players

    struct addrinfo condtns, *response;
    int status;             // for getaddrinfo
    int socketid;           // for the listening socket
    vector<int> newsocks{}; // for active sockets (1 for each thread)
    struct sockaddr_storage newconn;
    socklen_t newconn_size;

    memset(&condtns, 0, sizeof condtns);
    condtns.ai_family = AF_UNSPEC;     // both IPv4 and IPv6
    condtns.ai_socktype = SOCK_STREAM; // TCP
    condtns.ai_flags = AI_PASSIVE;     // fills in the IP for me - assign the address of the local host to the socket structures

    // response contains a linked list of structs that we will need later on
    // avoids the call to gethostbyname

    status = getaddrinfo(NULL, portnum.c_str(), &condtns, &response);

    if (status != 0)
    {
        cerr << "Error calling getaddrinfo: [ " << gai_strerror(status) << " ]" << endl;
        freeaddrinfo(response);
        return EXIT_FAILURE;
    }

    // we need to walk through all of the response linked list to check for valid entries
    struct addrinfo *res = response;
    while (res != NULL)
    {
        // opening the listening socket
        if ((socketid = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
            res = res->ai_next; // next struct in the linked list
            continue;
        }

        // bind the socket - since I used AI Passive it is binding the socket to the address of the host the server is running on
        if (bind(socketid, res->ai_addr, res->ai_addrlen) < 0)
        {
            res = res->ai_next;
            continue;
        }

        break;
    }

    if (res == NULL)
    {
        cerr << "Could not create and/or bind the socket" << endl;
        freeaddrinfo(response);
        return EXIT_FAILURE;
    }

    freeaddrinfo(response); // we do not need this anymore since we got the listening socket

    // listening for incoming connections
    if (listen(socketid, MAX_BACKLOG_SIZE) < 0)
    {
        cerr << "Error on listening for incoming connections: [ " << errno << " ]" << endl;
        return EXIT_FAILURE;
    }

    cout << "Awaiting connections..." << endl;

    newconn_size = sizeof newconn;
    while (true)
    {
        // accepting connections - creating a new thread for each connection
        int newsock = accept(socketid, (struct sockaddr *)&newconn, &newconn_size);
        if (newsock < 0)
        {
            close(socketid);
            cerr << "Error accepting the connection: [" << errno << " ]" << endl;
            continue; // wait for another connection to accept
        }

        if (num_players < MAX_PLAYERS)
        {
            m.lock();
            ++num_players; // increments the number of players
            m.unlock();

            Player *p = new Player(newsock);
            active_players.push_back(p);
            thread t(&Player::playerInitThread, p);
            player_thread_pool.push_back(t);
        }
        else
        {
            ++num_spectators;
            // thread t(spectator, newsock);
        }
    }

    return 0;
}