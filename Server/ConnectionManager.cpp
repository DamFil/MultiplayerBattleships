#include "ConnectionManager.h"

ConnManager::ConnManager(string portnum) : portnum(portnum), gameinfo(new GameState()), spectator(new Spectator(this->gameinfo)) {}
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
    //! PROBLEM FOR SEG FAULTS WHEN NUM_PLAYERS > 2 IS LIKELY HERE
    // start the turnRegulator thread
    thread turn_thread(&ConnManager::turnRegulator, this);
    int newsock;
    socklen_t newconn_size = sizeof newconn;
    while (gameinfo->getNumPlayers() < MAX_PLAYERS || !gameinfo->getStopConnect())
    {
        newsock = accept(this->socketid, (struct sockaddr *)&newconn, &newconn_size);
        // notifies the newly connected user that a game already started
        if (gameinfo->getStopConnect())
        {
            char sp = 'S';
            if (send(newsock, &sp, 1, 0) < 0)
            {
                cout << "Could not notify the player that the game already started..." << endl;
                close(newsock);
                // TODO Should wait for all threads to finish or just manually abort them...
                return failure;
            }
            break;
        }
        cout << "A new player connected..." << endl;

        Player *p = new Player(newsock, this->gameinfo);
        gameinfo->addPlayer(p);

        future<threadvalue> fu = async(launch::async, &Player::initPlayer, p);                // starts a new player's thread
        waiting_for_dc.push_back(thread(&ConnManager::waitForDisconnect, this, move(fu), p)); // creating the thread that waits for the player thread to finish
    }

    // accepting spectators
    gameinfo->addSpectator(newsock);
    while (true)
    {
        newsock = accept(this->socketid, (struct sockaddr *)&newconn, &newconn_size);
        char sp = 'S';
        if (send(newsock, &sp, 1, 0) < 0)
        {
            cout << "Could not notify the player that the game already started..." << endl;
            // TODO: Handle all active threads if there is a need
            return failure;
        }
        gameinfo->addSpectator(newsock);
    }

    turn_thread.join();

    return success;
}

void ConnManager::waitForDisconnect(future<threadvalue> fu, Player *p)
{
    threadvalue response = fu.get();

    switch (response)
    {
    case localerr:
        this->status = response;
        cout << "Error when communicating with: [ " << p->getName() << " ]..." << endl;
        gameinfo->removePlayer(p);
        break;
    case disconnected:
        // TODO: should broadcast to all active connections that the player disconnected
        this->status = response;
        cout << p->getName() << " disconnected..." << endl;
        gameinfo->removePlayer(p);
        break;
    case good:
        cout << p->getName() << " is setup and ready!" << endl;
        break;
    }

    return;
}

void ConnManager::turnRegulator()
{
    unique_lock<mutex> temp_locker(gameinfo->temp_lock);
    gameinfo->turns_start.wait(temp_locker, [this]()
                               { return this->gameinfo->getStopConnect(); });
    temp_locker.unlock(); // we do not need this lock anyway

    // searches by names rather than positions in the vector this ensures that no active player is skipped
    vector<string> names{};
    for (int i = 0; i < gameinfo->getNumPlayers(); i++)
    {
        names.push_back(gameinfo->getPlayer(i)->getName());
    }

    int i = 0;
    bool finished = false;
    while (!finished)
    {
        thread spectate_thread(&Spectator::spectateGame, spectator);
        Player *p = gameinfo->getPlayer(names.at(i)); // get player by name
        // in case the player disconnected in the meantime
        if (p == nullptr)
        {
            i = (i + 1) % names.size();
            continue;
        }
        future<threadvalue> attack = async(launch::async, &Player::startAttack, p);
        threadvalue response = attack.get();
        switch (response)
        {
        case localerr:
            this->status = response;
            cout << "Error when communicating with: [ " << p->getName() << " ]..." << endl;
            gameinfo->removePlayer(p);
            break;
        case disconnected:
            // TODO: should broadcast to all active connections that the player disconnected
            this->status = response;
            cout << p->getName() << " disconnected..." << endl;
            gameinfo->removePlayer(p);
            break;
        case lose:
            cout << p->getName() << " lost..." << endl;
            gameinfo->removePlayer(p);
            break;
        case win:
            cout << p->getName() << " won!" << endl;
            gameinfo->removePlayer(p);
            finished = true;
            break;
        }
        i = (i + 1) % names.size();
        // waits for the spectate thread to finish - this should actually not offer any waiting at all
        spectate_thread.join();
    }
    return;
}