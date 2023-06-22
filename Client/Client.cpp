#include "Client.h"

Client::Client(int socketid, string name) : socketid(socketid), name(name), p(new NewPlayer()) {}

Client::~Client()
{
    delete p;
}

clientvalue Client::initPlayer()
{
    // waiting for a receive signal - blocks until the thread sends it something
    char sg;
    int bytes_rec = recv(socketid, &sg, 1, 0);
    if (bytes_rec < 0)
    {
        cout << "Error communcating witht the server: [ " << errno << " ]" << endl;
        close(socketid);
        return clientvalue::localerr;
    }

    // first sending the length of the name - HEADER
    int name_len = name.length();
    int net_name_len = htonl(name_len); // converting to network byte ordering
    if (send(socketid, &net_name_len, sizeof(int), 0) < 0)
    {
        cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
        close(socketid);
        return clientvalue::localerr;
    }

    // sending the actual name
    const char *c_name = this->name.c_str();
    if (send(socketid, c_name, this->name.length(), 0) < 0)
        return localerr;
    // sendMessage(this->name); //! THIS FAILS
    // if (this->status != good)
    //    return this->status;

    // prompting the user to start the game
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
        return quit;
    }

    if (send(socketid, res.c_str(), 1, 0) < 0)
    {
        cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
        close(socketid);
        return localerr;
    }

    cout << "Time to set the positions of your ships!" << endl;

    // setting up the ships locally and as well making sure that the ships are correct
    initSendAllShips();
    if (this->status != good)
        return this->status;

    return good;
}

void Client::sendMessage(string message)
{
    // first sending the length of message - HEADER
    int msg_len = message.length();
    int net_msg_len = htonl(msg_len); // converting to network byte ordering
    if (send(socketid, &net_msg_len, sizeof(int), 0) < 0)
    {
        cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
        close(socketid);
        this->status = localerr;
        return;
    }

    // making sure that all of the bytes are sent
    int total_bytes_sent = 0;
    do
    {
        string rest_to_send = message.substr(total_bytes_sent); // takes the remaining, unsent part of the string
        int len_rest = rest_to_send.length();

        int bytes_sent = send(socketid, rest_to_send.c_str(), len_rest, 0);

        if (bytes_sent < 0)
        {
            cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
            close(socketid);
            this->status = localerr;
            return;
        }

        total_bytes_sent += bytes_sent;

    } while (total_bytes_sent != msg_len);
    this->status = good;
}

bool Client::parseCell(string cell, char *col, int *row)
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

void Client::initAndSendShip(ShipType t)
{
    char col;
    int row;
    char orient;
    string pos;
    int i = 0;

    do
    {
        if (i > 0)
            cout << "Please re-enter the ship's coordinates" << endl;

        switch (t)
        {
        case A:
            cout << "Please enter the starting position for the AirCraft Carrier:" << endl;
            break;
        case B:
            cout << "Please enter the starting position for the Battleship:" << endl;
            break;
        case C:
            cout << "Please enter the starting position for the Cruiser:" << endl;
            break;
        case D:
            cout << "Please enter the starting position for the Destroyer:" << endl;
            break;
        case S:
            cout << "Please enter the starting position for the Submarine:" << endl;
            break;
        }

        cin >> pos;
        while (!parseCell(pos, &col, &row))
        {
            cout << "The position of the ship has to be in the format COLROW where COL is the character of the colum (capital) and ROW is the number of the row" << endl;
            cin >> pos;
        }

        cout << "Please enter the orientation (H/V)" << endl;
        cin >> orient;
        while (orient != 'H' && orient != 'V')
        {
            cout << "You can only answer with H or V" << endl;
            cin >> orient;
        }
        ++i;
    } while (!p->newShip(t, col, row, orient));

    // sending the ships - col, row, orient
    // sending the col
    if (send(socketid, &col, 1, 0) < 0)
    {
        cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
        close(socketid);
        this->status = localerr;
    }

    // sending the row
    int nrow = htonl(row);
    if (send(socketid, &nrow, sizeof(int), 0) < 0)
    {
        cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
        close(socketid);
        this->status = localerr;
    }

    // sending the orientation
    if (send(socketid, &orient, 1, 0))
    {
        cerr << "Error communcating with the server: [ " << errno << " ]" << endl;
        close(socketid);
        this->status = localerr;
    }

    this->status = good;
    return;
}

void Client::initSendAllShips()
{
    initAndSendShip(A);
    if (status != good)
        return;
    initAndSendShip(B);
    if (status != good)
        return;
    initAndSendShip(C);
    if (status != good)
        return;
    initAndSendShip(D);
    if (status != good)
        return;
    initAndSendShip(D);
    if (status != good)
        return;
    initAndSendShip(S);
    if (status != good)
        return;
    initAndSendShip(S);
    if (status != good)
        return;

    this->status = good;
}
