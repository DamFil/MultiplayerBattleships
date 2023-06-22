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

bool parseCell(string cell, char *col, int *row)
{
    if (cell.length() < 2 || cell.length() > 3)
        return false;
    if (!(isalpha(cell.at(0))) && (cell.at(0) < 'A' || cell.at(0) > 'J'))
        return false;
    if (!isdigit(cell.at(1)))
        return false;
    if (cell.length() == 3 && !isdigit(cell.at(2)))
        return false;

    *col = cell.at(0);
    *row = stoi(cell.substr(1));

    return true;
}

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
    string portnum = iport.substr(poscol + 1);
    string name = argv[2];

    cout << "ip: [ " << ip << " ]" << endl;
    cout << "port number: [ " << portnum << " ]" << endl;
    cout << "name: [ " << name << " ]" << endl;

    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SETTING UP THE CONNECTION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    struct addrinfo condtns, *server, *ser; // server contains information about the server we are connecting to
    int status;                             // for getaddrinfo
    int socketid;                           // for the listening socket

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

    // waiting for a receive signal
    char sg;
    int bytes_rec = recv(socketid, &sg, 1, MSG_WAITALL); // just blocks until the server thread starts

    // first sending the length of the name - HEADER
    int name_len = name.length();
    int net_name_len = htonl(name_len); // converting to network byte ordering
    if (send(socketid, &net_name_len, sizeof(int), 0) < 0)
    {
        cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
        close(socketid);
        return EXIT_FAILURE;
    }

    // making sure that all of the bytes are sent
    int total_bytes_sent = 0;
    do
    {
        string rest_to_send = name.substr(total_bytes_sent); // takes the remaining, unsent part of the string
        int len_rest = rest_to_send.length();

        int bytes_sent = send(socketid, rest_to_send.c_str(), len_rest, 0);

        if (bytes_sent < 0)
        {
            cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
            close(socketid);
            return EXIT_FAILURE;
        }

        total_bytes_sent += bytes_sent;

    } while (total_bytes_sent != name_len);

    // sending whether you want to start the game or not
    string res;
    cout << "Do you want to start the game? (Y/Q)" << endl;
    cin >> res;
    while (res != "Y" && res != "Q")
    {
        cout << "You can only answer with 'Y' or 'Q'" << endl;
        cin >> res;
    }

    if (res == "Q")
    {
        close(socketid);
        return EXIT_SUCCESS;
    }

    int bytes_sent = send(socketid, res.c_str(), 1, 0);

    cout << "Time to set the positions of your ships!" << endl;

    string ac;
    cout << "Please enter the starting position for the AirCraft Carrier:" << endl;
    cin >> ac;

    return 0;
}