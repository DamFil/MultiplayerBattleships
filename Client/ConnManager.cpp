#include "ConnManager.h"

ConnManager::ConnManager(string ip, string portnum) : ip(ip), portnum(portnum) {}

ConnManager::~ConnManager()
{
    closeSocket();
}

int ConnManager::setupAndConnect()
{
    memset(&condtns, 0, sizeof condtns);
    condtns.ai_family = AF_UNSPEC;     // both IPv4 and IPv6
    condtns.ai_socktype = SOCK_STREAM; // TCP

    int status = getaddrinfo(ip.c_str(), portnum.c_str(), &condtns, &server); // gets the server info

    if (status != 0)
    {
        cerr << "Error calling getaddrinfo: [ " << gai_strerror(status) << " ]" << endl;
        freeaddrinfo(server);
        return -1;
    }

    auto ser = server;
    while (ser != NULL)
    {
        this->socketid = socket(ser->ai_family, ser->ai_socktype, ser->ai_protocol);
        if (socketid < 0)
        {
            ser = ser->ai_next;
            continue;
        }

        if (connect(socketid, ser->ai_addr, ser->ai_addrlen) < 0)
        {
            ser = ser->ai_next;
            continue;
        }

        break;
    }

    if (ser == NULL)
    {
        cerr << "Could not establish connection between the client and the server" << endl;
        freeaddrinfo(server);
        return -1;
    }

    freeaddrinfo(server);

    cout << "Connected to server..." << endl;

    return 0;
}

int ConnManager::getSocket()
{
    return this->socketid;
}

void ConnManager::closeSocket()
{
    close(this->socketid);
}