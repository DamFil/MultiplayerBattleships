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
        cout << "Failed receiving the thread start signal..." << endl;
        return clientvalue::localerr;
    }

    // first sending the length of the name - HEADER
    int name_len = name.length();
    int net_name_len = htonl(name_len); // converting to network byte ordering
    if (send(socketid, &net_name_len, sizeof(int), 0) < 0)
    {
        cerr << "Failed sending the name length..." << endl;
        close(socketid);
        return clientvalue::localerr;
    }

    // sending the actual name
    const char *c_name = this->name.c_str();
    if (send(socketid, c_name, this->name.length(), 0) < 0)
    {
        cout << "Failed sending the name..." << endl;
        return localerr;
    }
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
        return quit;

    if (send(socketid, res.c_str(), 1, 0) < 0)
    {
        cerr << "Failed sending 'Y'..." << endl;
        return localerr;
    }

    return good;
}

void Client::sendMessage(string message)
{
    // first sending the length of message - HEADER
    int msg_len = message.length();
    int net_msg_len = htonl(msg_len); // converting to network byte ordering
    if (send(socketid, &net_msg_len, sizeof(int), 0) < 0)
    {
        cerr << "Failed sending the message header..." << endl;
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
            cerr << "Failed sending the message..." << endl;
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
    int length = cell.length();
    if (length < 2 || length > 3)
        return false;
    if (!(isalpha(cell.at(0))) && (cell.at(0) < 'A' || cell.at(0) > 'J'))
        return false;
    if (!isdigit(cell.at(1)))
        return false;
    if (length == 3 && !isdigit(cell.at(2)))
        return false;

    *col = cell.at(0);
    *row = stoi(cell.substr(1));

    if (*row < 1 || *row > 10)
        return false;

    return true;
}

clientvalue Client::sendShip(char col, int row, char orientation)
{
    if (send(socketid, &col, 1, 0) < 0)
    {
        cerr << "Failed sending column char..." << endl;
        return localerr;
    }

    // sending the row
    int nrow = htonl(row);
    if (send(socketid, &nrow, sizeof(int), 0) < 0)
    {
        cerr << "Failed sending row number..." << endl;
        return localerr;
    }

    // sending the orientation
    if (send(socketid, &orientation, 1, 0) < 0)
    {
        cerr << "Failed sending ship orientation..." << endl;
        return localerr;
    }

    return good;
}

void Client::initShip(ShipType t, char *col, int *row, char *orientation)
{
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
        while (!parseCell(pos, col, row))
        {
            cout << "The position of the ship has to be in the format COLROW where COL is the character of the colum (capital) and ROW is the number of the row" << endl;
            cin >> pos;
        }

        cout << "Please enter the orientation (H/V)" << endl;
        cin >> *orientation;
        while (*orientation != 'H' && *orientation != 'V')
        {
            cout << "You can only answer with H or V" << endl;
            cin >> *orientation;
        }
        ++i;
    } while (!p->newShip(t, *col, *row, *orientation)); // this while loop checks boundaries on the grid and collisions with other ships and adds the ship locally
}

clientvalue Client::initAllShips()
{
    char col;
    int row;
    char orientation;

    vector<ShipType> sts{A, B, C, D, D, S, S};

    for (auto t : sts)
    {
        initShip(t, &col, &row, &orientation);
        this->status = sendShip(col, row, orientation);
        if (this->status != good)
            break;
    }

    return this->status;
}

bool Client::addShipLocal(ShipType t, char col, int row, char orientation)
{
    return p->newShip(t, col, row, orientation);
}

clientvalue Client::attack()
{
    char sg;
    if (recv(this->socketid, &sg, 1, MSG_WAITALL) < 0) //! this does not wait for some reason
    {
        cout << "Wait to receive signal for starting the attack..." << endl;
        return localerr;
    }

    cout << "Starting the attack!" << endl;
    return good;
}
