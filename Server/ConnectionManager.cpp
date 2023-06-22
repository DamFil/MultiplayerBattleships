#include "ConnectionManager.h"

ConnManager::ConnManager(string portnum) : portnum(portnum), gameinfo(new GameState()) {}
ConnManager::~ConnManager()
{
    delete this->gameinfo;
}

Output ConnManager::setupAndListen()
{
    struct addrinfo *response;

    // setting up the condtns struct
    memset(&condtns, 0, sizeof condtns);
    condtns.ai_family = AF_UNSPEC;     // both IPv4 and IPv6
    condtns.ai_socktype = SOCK_STREAM; // TCP
    condtns.ai_flags = AI_PASSIVE;     // fills in the IP for me - assign the address of the local host to the socket structures

    int status = getaddrinfo(NULL, portnum.c_str(), &condtns, &response); // filling in response
    if (status != 0)
    {
        cerr << "Error calling getaddrinfo: [ " << gai_strerror(status) << " ]" << endl;
        freeaddrinfo(response);
        return failure;
    }

    // we need to walk through all of the response linked list to check for valid entries
    struct addrinfo *res = response;
    while (res != NULL)
    {
        // opening the listening socket
        if ((socketid = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
            res = res->ai_next;
            continue;
        }

        // bind the socket - since I used AI Passive it is binding the socket to localhost
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
        return failure;
    }

    freeaddrinfo(response); // we do not need this anymore since we got the listening socket

    // listening for incoming connections
    if (listen(socketid, MAX_BACKLOG_SIZE) < 0)
    {
        cerr << "Error on listening for incoming connections: [ " << errno << " ]" << endl;
        return failure;
    }

    cout << "Listening for connections..." << endl;
    return success;
}

Output ConnManager::acceptConnections()
{
    while (gameinfo->getNumPlayers() < MAX_PLAYERS)
    {
        socklen_t newconn_size = sizeof newconn;
        int newsock = accept(this->socketid, (struct sockaddr *)&newconn, &newconn_size);

        Player *p = new Player(newsock, this->gameinfo);
        gameinfo->addPlayer(p);

        futures.push_back(async(launch::async, &Player::getNameAndStart, p)); // starts a new player's thread
        thread t(waitForDisconnect, futures.back());                          // this thread handles disconnects
    }
}

void ConnManager::waitForDisconnect(future<threadvalue> &fu, Player *p)
{
    threadvalue response = fu.get();
    if (response == localerr || response == disconnected)
    {
        cout << "Player: [ " << p->getName() << " ] disconnected..." << endl;
        gameinfo->removePlayer(p);
    }

    return;
}