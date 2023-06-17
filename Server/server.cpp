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

using namespace std;

#define MAX_PLAYERS 4
#define MAX_SPECTATORS 10
#define MAX_BACKLOG_SIZE 10

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

    int num_players = 0; // keeps track of the number of connected players
    struct addrinfo condtns, *response;
    int status;             // for getaddrinfo
    int socketid;           // for the listening socket
    vector<int> newsocks{}; // for active sockets (1 for each thread)
    struct sockaddr_storage newconn;
    socklen_t newconn_size;

    memset(&condtns, 0, sizeof(condtns));
    condtns.ai_family = AF_UNSPEC;     // both IPv4 and IPv6
    condtns.ai_socktype = SOCK_STREAM; // TCP
    condtns.ai_flags = AI_PASSIVE;     // fills in the IP for me - assign the address of the local host to the socket structures

    // response contains a linked list of structs that we will need later on
    // avoids the call to gethostbyname
    if ((status = getaddrinfo(NULL, portnum.c_str(), &condtns, &response)) != 0)
    {
        cerr << "Error calling getaddrinfo: [ " << gai_strerror(status) << " ]" << endl;
        freeaddrinfo(response);
        exit(EXIT_FAILURE);
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

    if (socketid < 0)
    {
        cerr << "Could not create and/or bind the socket" << endl;
        freeaddrinfo(response);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(response); // we do not need this anymore since we got the listening socket

    // listening for incoming connections
    if (listen(socketid, MAX_BACKLOG_SIZE) < 0)
    {
        cerr << "Error on listening for incoming connections: [ " << errno << " ]" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Awaiting connections..." << endl;

    while (true)
    {
        // accepting connections - creating a new thread for each connection
        newconn_size = sizeof newconn;
        int newsock;
        if ((newsock = accept(socketid, (struct sockaddr *)&newconn, &newconn_size)) < 0)
        {
            cerr << "Error accepting the connection: [" << errno << " ]" << endl;
            continue; // wait for another connection to accept
        }
    }

    return 0;
}