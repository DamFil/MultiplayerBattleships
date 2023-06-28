#include "Client.h"

Client::Client(int socketid, string name) : sockfd(socketid), name(name), p(new NewPlayer()) {}

Client::~Client()
{
    delete p;
}

clientvalue Client::initPlayer()
{
    // waiting for a receive signal - blocks until the thread sends it something
    char sg = recvChar();
    if (this->status != good)
    {
        cout << "Failed receiving the start signal..." << endl;
        return this->status;
    }

    // checking if the player is a spectator or not
    // call the spectator function
    if (sg == 'S')
    {
        cout << "A game is currently in progress... You are going to spectate the ongoing game!" << endl;
        return spectate();
    }

    // sending the name of the player
    sendMessage(this->name);
    if (this->status != good)
        return this->status;

    // prompting the user to start the game
    char res;
    cout << "Do you want to start the game? (Y/Q)" << endl;
    cin >> res;
    while (res != 'Y' && res != 'Q')
    {
        cout << "You can only answer with Y or Q" << endl;
        cin >> res;
    }

    if (res == 'Q')
        return quit;

    sendMessage(res);
    if (this->status != good)
    {
        cout << "Failed sending the start game ans" << endl;
        return this->status;
    }

    return good;
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

void Client::sendShip(char col, int row, char orientation)
{
    // sending the column
    sendMessage(col);
    if (this->status != good)
        return;

    // sending the row
    sendInt(row);
    if (this->status != good)
        return;

    // sending the orientation
    sendMessage(orientation);
    if (this->status != good)
        return;

    this->status = good;
    return;
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
        sendShip(col, row, orientation);
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
        cout << "Waiting for other players to finish setting up and attacking" << endl;

        // receiving the start attack signal
        char start_msg = recvChar();
        if (this->status != good)
        {
            cout << "Failed receiving start attack signal" << endl;
            return this->status;
        }

        if (start_msg == 'S')
        {
            cout << "You lost the game...All your ships have been sunk!" << endl;
            return lose;
        }
        else if (start_msg == 'W')
        {
            cout << "Congratualations, you won!" << endl;
            return good;
        }

        // receiving the attempts accumulated over the other players' turns
        vector<tuple<char, int, char>> my_attemps = recvAllAttempts();
        if (this->status != good)
        {
            cout << "Failed receiving my attempts..." << endl;
            return this->status;
        }
        for (auto att : my_attemps)
        {
            p->addAttempt(att);
        }
        // prints my map
        p->showMap();

        // receiving the attempts of all other players and printing their maps
        vector<string> names;
        vector<vector<tuple<char, int, char>>> opp_maps;
        int num_opponents = recvInt();
        if (this->status != good)
        {
            cout << "Could not recv num of opponents" << endl;
            return this->status;
        }

        for (int i = 0; i < num_opponents; i++)
        {
            names.push_back(recvMessage());
            if (this->status != good)
            {
                cout << "Failed recv opp name..." << endl;
                return this->status;
            }

            vector<tuple<char, int, char>> foreign_attempts = recvAllAttempts();
            opp_maps.push_back(foreign_attempts);
            if (this->status != good)
            {
                cout << "Failed recv opp attempts" << endl;
                return this->status;
            }

            cout << names.back() << "' map:\n"
                 << endl;
            p->showMap(foreign_attempts);
        }

        cout << "Starting the attack!" << endl;

        // choosing the player to attack and sending the name to the server
        choosePlayerToAttack(names);
        if (this->status != good)
        {
            cout << "Failed sending the name of the opponent..." << endl;
            return this->status;
        }

        // sending the cell you want to bomb
        sendStrike();
        if (this->status != good)
            return this->status;

        // receive the result of the strike you just sent out
        char res = recvChar();
        if (this->status != good)
        {
            cout << "Could not get the result of your strike..." << endl;
            return this->status;
        }

        // TODO: We can possibly change this to send the attempt when we do ncurses
        if (res == 'X')
            cout << "You hit a ship!" << endl;
        else
            cout << "You missed!" << endl;

        cout << "You finished the attacking round" << endl;
    }
}

void Client::recvAttempt(char *col, int *row, char *hm)
{
    // receiving the column
    char new_col = recvChar();
    if (this->status != good)
        return;
    *col = new_col;

    // receiving the row
    int new_row = recvInt();
    if (this->status != good)
        return;
    *row = new_row;

    // receiving the hit/miss
    char new_hm = recvChar();
    if (this->status != good)
        return;
    *hm = new_hm;

    this->status = good;
    return;
}

vector<tuple<char, int, char>> Client::recvAllAttempts()
{
    // receiving the length of the vector
    int num_of_attempts = recvInt();
    if (this->status != good)
        return {};

    vector<tuple<char, int, char>> received_attempts{};
    for (int i = 0; i < num_of_attempts; i++)
    {
        char col, hm;
        int row;
        recvAttempt(&col, &row, &hm);
        if (this->status != good)
            return {};
        received_attempts.push_back(make_tuple(col, row, hm));
    }

    this->status = good;
    return received_attempts;
}

void Client::choosePlayerToAttack(vector<string> names)
{
    string name;
    cout << "Enter the name of the player you want to attack..." << endl;
    bool found = false;
    do
    {
        cin >> name;
        for (auto n : names)
        {
            if (name != n)
                continue;
            found = true;
            break;
        }
    } while (!found);

    sendMessage(name);
    return;
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

    sendMessage(col);
    if (this->status != good)
        return;

    sendInt(row);
    if (this->status != good)
        return;

    this->status = good;
    return;
}

void Client::checkBytesRec(int bytes_rec)
{
    if (bytes_rec < 0)
    {
        this->status = localerr;
        return;
    }
    else if (bytes_rec == 0)
    {
        this->status = disconnected;
        return;
    }

    this->status = good;
    return;
}

void Client::sendMessage(string msg)
{
    // sending msg length
    sendInt(msg.length());
    if (this->status != good)
        return;

    // sending the actual msg
    if (send(sockfd, msg.c_str(), msg.length(), 0) < 0)
    {
        cout << "Failed sending the msg: " << msg << "..." << endl;
        this->status = localerr;
        return;
    }

    this->status = good;
    return;
}

void Client::sendMessage(char msg)
{
    if (send(sockfd, &msg, 1, 0) < 0)
    {
        this->status = localerr;
        return;
    }

    this->status = good;
    return;
}

string Client::recvMessage()
{
    // receving the length
    int header = recvInt();
    if (this->status != good)
        return "";

    // receiving the actual message
    char buf[header];
    int bytes_rec = recv(sockfd, buf, header, MSG_WAITALL);
    checkBytesRec(bytes_rec);
    if (this->status != good)
        return "";

    return string(buf).substr(0, header);
}

char Client::recvChar()
{
    char received;
    int bytes_rec = recv(sockfd, &received, 1, MSG_WAITALL);
    checkBytesRec(bytes_rec);
    if (this->status != good)
        return ERROR_CHAR;

    this->status = good;
    return received;
}

void Client::sendInt(int x)
{
    int to_send = htonl(x);
    if (send(sockfd, &to_send, sizeof(int), 0) < 0)
    {
        cout << "Failed sending the integer: " << x << "..." << endl;
        this->status = localerr;
        return;
    }

    this->status = good;
    return;
}

int Client::recvInt()
{
    int received;
    int bytes_rec = recv(sockfd, &received, sizeof(int), 0);
    checkBytesRec(bytes_rec);
    if (this->status != good)
        return ERROR_INT;

    return ntohl(received);
}

clientvalue Client::spectate()
{
    while (true)
    { // receiving the status of the game
        char game_state = recvChar();
        if (this->status != good)
        {
            cout << "Could not receive the state of the game..." << endl;
            return this->status;
        }

        if (game_state == 'F')
        {
            cout << "The game finished... You will be disconnected from the server..." << endl;
            break;
        }

        int num_players = recvInt();
        if (this->status != good)
        {
            cout << "Could not get number of players..." << endl;
            return this->status;
        }

        for (int i = 0; i < num_players; i++)
        {
            // receiving the ships
            vector<tuple<char, int, char>> foreign_ships = recvAllShips();
            if (this->status != good)
            {
                cout << "Could not receive the ship positions..." << endl;
                return this->status;
            }

            // receiving the attempts of the player
            vector<tuple<char, int, char>> foreign_attempts = recvAllAttempts();
            if (this->status != good)
            {
                cout << "Could not receive the attempts..." << endl;
                return this->status;
            }

            // TODO I need to print the above information
            NewPlayer *in_game = new NewPlayer();
            vector<ShipType> ship_order{A, B, C, D, D, S, S};
            for (int i = 0; i < foreign_ships.size(); i++)
            {
                in_game->newShip(ship_order[i], get<0>(foreign_ships[i]), get<1>(foreign_ships[i]), get<2>(foreign_ships[i]));
            }

            for (auto att : foreign_attempts)
            {
                in_game->addAttempt(att);
            }
            in_game->showMap();
            delete in_game; // TODO I should handle this in a smarter way using caching
        }
    }

    return good;
}

vector<tuple<char, int, char>> Client::recvAllShips()
{
    vector<tuple<char, int, char>> ships{};
    // since there are 7 ships in total
    for (int i = 0; i < 7; i++)
    {
        char col, orient;
        int row;
        recvAttempt(&col, &row, &orient);
        if (this->status != good)
            return {};
        ships.push_back(make_tuple(col, row, orient));
    }

    this->status = good;
    return ships;
}
