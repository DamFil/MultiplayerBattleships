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

    // checking if the player is a spectator or not
    if (sg == 'S')
    {
        // TODO Run the spectator routine
        cout << "A game is currently in progress... You will be disconnected form the server!" << endl;
        return quit;
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
    while (true)
    {
        cout << "Waiting for other players to finish their attacking turn..." << endl;
        // receiving the start attack signal
        char start_attack;
        int bytes_rec;
        if ((bytes_rec = recv(this->socketid, &start_attack, 1, MSG_WAITALL)) < 0)
        {
            cout << "Failed to receive signal for starting the attack..." << endl;
            return localerr;
        }

        if (start_attack != 'A' || bytes_rec == 0)
        {
            cout << "Incorrect start_attack value..." << endl;
            return disconnected;
        }

        // receiving the attempts accumulated over the other players' turns
        vector<tuple<char, int, char>> my_attemps = recvAllAttempts();
        if (this->status != good)
            return this->status;

        // updating my map based on other players' attempts
        for (auto att : my_attemps)
        {
            p->addAttempt(att);
        }

        cout << "Starting the attack!" << endl;

        // receiving the names length
        int tmp;
        bytes_rec = recv(socketid, &tmp, sizeof(int), MSG_WAITALL);
        if (bytes_rec < 0)
        {
            cout << "Local error when receiving the header for the names..." << endl;
            return localerr;
        }
        else if (bytes_rec == 0)
        {
            cout << "Local error when receiving the header for the names..." << endl;
            return disconnected;
        }
        int header = ntohl(tmp);

        // receiving the actual names
        char buf[header];
        bytes_rec = recv(socketid, &buf, header, MSG_WAITALL);
        if (bytes_rec < 0)
        {
            cout << "Local error when receiving the names..." << endl;
            return localerr;
        }
        else if (bytes_rec == 0)
        {
            cout << "Disconnection prior to receiving the names..." << endl;
            return disconnected;
        }
        string names = string(buf).substr(0, header);

        // choosing the player to attack and sending the name to the server
        choosePlayerToAttack(names);
        if (this->status != good)
            return this->status;

        // receiving the attempts of the player so that you can print it
        vector<tuple<char, int, char>> foreign_attempts = recvAllAttempts();
        if (this->status != good)
            return this->status;

        // Printing my map and the attempts of the foreign player
        p->showMap(foreign_attempts);

        // sending the cell you want to bomb
        sendStrike();
        if (this->status != good)
            return this->status;

        // receive the result of the strike you just sent out
        char col, hm;
        int row;
        recvAttempt(&col, &row, &hm); //! this isn't receiving the correct row and hm values
        if (this->status != good)
            return this->status;

        foreign_attempts.push_back(make_tuple(col, row, hm));
        p->showMap(foreign_attempts);

        cout << "You finished the attacking round" << endl;
    }
}

void Client::recvAttempt(char *col, int *row, char *hm)
{
    // receiving the column
    int tmp;
    char col_new, hm_new;
    int row_new;

    int bytes_rec = recv(socketid, &col_new, 1, MSG_WAITALL);
    if (bytes_rec < 0)
    {
        cout << "Could not receive the column of the attempt..." << endl;
        this->status = localerr;
        return;
    }
    else if (bytes_rec == 0)
    {
        cout << "Could not receive the column of the attempt..." << endl;
        this->status = disconnected;
        return;
    }

    // receiving the row
    bytes_rec = recv(socketid, &tmp, sizeof(int), MSG_WAITALL);
    if (bytes_rec < 0)
    {
        cout << "Could not receive the row of the attempt..." << endl;
        this->status = localerr;
        return;
    }
    else if (bytes_rec == 0)
    {
        cout << "could not receive the row of the attempt..." << endl;
        this->status = disconnected;
        return;
    }
    row_new = ntohl(tmp);

    // receiving the hit/miss
    bytes_rec = recv(socketid, &hm_new, 1, MSG_WAITALL);
    if (bytes_rec < 0)
    {
        cout << "Could not receive the column of the attempt..." << endl;
        this->status = localerr;
        return;
    }
    else if (bytes_rec == 0)
    {
        cout << "Could not receive the column of the attempt..." << endl;
        this->status = disconnected;
        return;
    }

    *col = col_new;
    *row = row_new;
    *hm = hm_new;

    this->status = good;
    return;
}

vector<tuple<char, int, char>> Client::recvAllAttempts()
{
    // receiving the length of the vector
    int tmp;
    int bytes_rec = recv(socketid, &tmp, sizeof(int), 0);
    if (bytes_rec < 0)
    {
        cout << "Local error when receiving the length of the grid of the player you want to attack..." << endl;
        this->status = localerr;
        return {};
    }
    else if (bytes_rec == 0)
    {
        cout << "Disconnected from the server when receiving length of the grid..." << endl;
        this->status = disconnected;
        return {};
    }
    int num_of_attempts = ntohl(tmp);

    vector<tuple<char, int, char>> received_attempts{};
    for (int i = 0; i < num_of_attempts; i++)
    {
        char col, hm;
        int row;
        recvAttempt(&col, &row, &hm);
        if (this->status != good)
            return {};
        auto att = make_tuple(col, row, hm);
        received_attempts.push_back(att);
    }

    this->status = good;
    return received_attempts;
}

bool Client::hasSpace(string word)
{
    for (int i = 0; i < word.length(); i++)
    {
        if (word[i] == ' ')
            return true;
    }

    return false;
}

void Client::choosePlayerToAttack(string names)
{
    // splits the string names using space as the delimiter
    // taken from: https://sentry.io/answers/split-string-in-cpp/
    vector<string> word_names{};
    int pos = 0;
    while (pos < names.length())
    {
        pos = names.find(" ");
        word_names.push_back(names.substr(0, pos));
        names.erase(0, pos + 1);
    }

    cout << "Please choose the player you want to attack:" << endl;
    for (int i = 0; i < word_names.size(); i++)
    {
        cout << i + 1 << ". " << word_names[i] << endl;
    }

    // choosing the player to attack
    int choice;
    cin >> choice;

    while (cin.fail() || choice < 1 || choice > word_names.size())
    {
        cin.clear();
        cout << "You must choose between the presented options..." << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clearing the input buffer
        cin >> choice;
    }

    string name_choice = word_names.at(choice - 1);

    // sending the length of the name
    int header = htonl(name_choice.length());
    if (send(socketid, &header, sizeof(int), 0) < 0)
    {
        cout << "Failure sending the length of the name of the player to attack..." << endl;
        this->status = localerr;
        return;
    }

    // sending the actual player's name
    if (send(socketid, name_choice.c_str(), name_choice.length(), 0) < 0)
    {
        cout << "Failure sending the name of the player to attack..." << endl;
        this->status = localerr;
        return;
    }
}

void Client::sendStrike()
{
    string cell;
    char col;
    int row;

    cout << "What cell would you like to bomb? (COLROW)" << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clearing the input buffer
    cin >> cell;

    while (!parseCell(cell, &col, &row))
    {
        cout << "The cell you entered is incorrect. Make sure it follows the COLROW format..." << endl;
        cin >> cell;
    }

    if (send(socketid, &col, 1, 0) < 0)
    {
        cout << "Failed sending the attempt cell column..." << endl;
        this->status = localerr;
        return;
    }

    int nrow = htonl(row);
    if (send(socketid, &nrow, sizeof(int), 0) < 0)
    {
        cout << "Failed sending the attempt cell row..." << endl;
        this->status = localerr;
        return;
    }

    this->status = good;
    return;
}
