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

using namespace std;

int main(int argc, char *argv[])
{
    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PARSING COMMAND LINE ARGUMENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 1) battleships 2) ip:portnum 3) player's name
    if (argc != 3)
    {
        cout << "Usage: battleships ip:portnum name" << endl;
        return EXIT_SUCCESS;
    }

    string iport = argv[1];
    size_t poscol = iport.find_first_of(':', 6); // 6 because the minimum length ip is 7 chars long (1.1.1.1)
    if (poscol == string::npos)
    {
        cout << "The first argument has to be ip:portnum. For example: 192.168.2.3:5001" << endl;
        return EXIT_SUCCESS;
    }

    string ip = iport.substr(0, poscol);
    string portnum = iport.substr(poscol);
    string name = argv[2];

    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SETTING UP THE CONNECTION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    struct addrinfo condtns, *server, *ser; // server contains information about the server we are connecting to
    int status;                             // for getaddrinfo
    int socketid;                           // for the listening socket
    vector<int> newsocks{};                 // for active sockets (1 for each thread)
    struct sockaddr_storage newconn;
    socklen_t newconn_size;

    memset(&condtns, 0, sizeof condtns);
    condtns.ai_family = AF_UNSPEC;     // both IPv4 and IPv6
    condtns.ai_socktype = SOCK_STREAM; // TCP

    status = getaddrinfo(ip.c_str(), portnum.c_str(), &condtns, &server); // gets the server info

    if (status != 0)
    {
        cerr << "Error calling getaddrinfo: [ " << gai_strerror(status) << " ]" << endl;
        freeaddrinfo(server);
        return EXIT_FAILURE;
    }

    ser = server;
    while (ser != NULL)
    {
        socketid = socket(ser->ai_family, ser->ai_socktype, ser->ai_protocol);
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
        return EXIT_FAILURE;
    }

    freeaddrinfo(server);

    cout << "Connected to server..." << endl;

    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SENDING/RECEIVING TO/FROM THE CLIENT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    return 0;
}