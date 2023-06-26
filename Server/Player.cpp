#include "Player.h"
#include "GameState.h"

using namespace std;

Player::Player(int sockfd, GameState *gameinfo) : sockfd(sockfd), gameinfo(gameinfo),
                                                  status(threadvalue::good), name(""), ready(false), lost(false) {}

Player::~Player()
{
    lock_guard<mutex> l(this->player_mutex);
    close(this->sockfd);
}

threadvalue Player::getNameAndStart()
{
    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GETTING THE NAME AND GAME START CONFIRMATION OF THE PLAYER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // client waits for this signal to start sending info
    char st = 'P';
    bytes_sent = send(this->sockfd, &st, 1, 0);
    if (bytes_sent < 0)
    {
        cout << "Failed notifying the client to start..." << endl;
        return localerr;
    }

    // getting the header
    if (recv(this->sockfd, &header, sizeof(header), 0) < 0)
    {
        cout << "Failed receiving the name length (HEADER)..." << endl;
        return localerr;
    }
    int name_len = ntohl(this->header); // converting to server byte ordering

    // reading the actual name
    char buf[name_len];
    bytes_rec = recv(this->sockfd, buf, name_len, MSG_WAITALL); // waiting for all the bytes
    threadvalue res = checkBytesRec();
    if (res != threadvalue::good)
    {
        cout << "Failed receiving the name of the player..." << endl;
        return res;
    }

    // have to do this because temp_name might contain some unwanted characters
    string temp_name = buf;
    this->name = temp_name.substr(0, name_len);

    // getting the answer for starting the game
    char sg;
    bytes_rec = recv(this->sockfd, &sg, sizeof(char), MSG_WAITALL); // sizeof char == 1 byte
    res = checkBytesRec();
    if (res != threadvalue::good)
    {
        cout << "Failed receiving start game response..." << endl;
        return res;
    }

    if (sg != 'Y')
        return localerr;

    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GETTING SHIPS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    for (int i = 0; i < MAX_SHIPS; i++)
    {
        addShip();
        if (this->status != good)
            return this->status;
    }
    this->ready = true;

    // we have to wait for all the players to finish their initialization (there must be 2 or more players)
    while (gameinfo->getNumPlayers() < 2 || !gameinfo->getStartGame())
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ATTACKING LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    while (true)
    {
        unique_lock<mutex> turn_locker(gameinfo->turn_lock);
        gameinfo->turn_notifier.wait(turn_locker, [this]
                                     { return this->attack; }); // this will only unlock when the turn selector thread finishes setting the turn

        // first checks if it lost from the previous players attacks
        if (checkIfLost())
        {
            this->lost = true;
            turn_locker.unlock();
            return good;
        }

        // terminates if it won - it will only detect if it won at the start of its turn
        // this is because each player is responsible itself for detecting when it lost
        if (gameinfo->getNumPlayers() == 1)
        {
            turn_locker.unlock();
            return good;
        }

        char sa = 'A';
        if (send(sockfd, &sa, 1, 0) < 0)
        {
            cout << "Failed sending the attack start signal..." << endl;
            return localerr;
        }

        //  Sending the attempts this player accumulated over the last turn
        // TODO: You should change this so it just sends the new attempts rather than the whole thing again
        sendAllAttempts(this);
        if (this->status != good)
            return this->status;

        string names = gameinfo->getNames(this);

        // send the header and the names
        header = htonl(names.length());
        if (send(sockfd, &header, sizeof(int), 0) < 0)
        {
            cout << "Failed sending the names header" << endl;
            return localerr;
        }

        if (send(sockfd, names.c_str(), names.length(), 0) < 0)
        {
            cout << "Failed sending the names of the remaining players" << endl;
            return localerr;
        }

        // now we need to receive the length of the player's name and the player's name itself
        bytes_rec = recv(sockfd, &tmp, sizeof(int), MSG_WAITALL);
        res = checkBytesRec();
        if (res != good)
        {
            cout << "Failed getting the player-to-be-attacked name header..." << endl;
            return res;
        }
        header = ntohl(tmp);

        bytes_rec = recv(sockfd, buf, header, MSG_WAITALL);
        res = checkBytesRec();
        if (res != good)
        {
            cout << "Failed getting the player-to-be-attacked name..." << endl;
            return res;
        }
        string to_be_attacked_name = string(buf).substr(0, header);

        Player *to_attack = gameinfo->getPlayer(to_be_attacked_name);

        /*
         before getting the cell we need to send all of the attempts of that player
         so that the attacker knows the hits and the misses
        */

        sendAllAttempts(to_attack);
        if (this->status != good)
        {
            cout << "Failed at sending the attacked player's attempts..." << endl;
            return localerr;
        }

        // now we need to receive the cell that we are bombing - col row format
        char col;
        bytes_rec = recv(sockfd, &col, 1, MSG_WAITALL);
        res = checkBytesRec();
        if (res != good)
        {
            cout << "Failed getting the column of the cell to attack..." << endl;
            return res;
        }

        int row;
        bytes_rec = recv(sockfd, &tmp, sizeof(int), MSG_WAITALL);
        res = checkBytesRec();
        if (res != good)
        {
            cout << "Failed getting the row of the cell to attack..." << endl;
            return res;
        }
        row = ntohl(tmp);

        // we need to add that attempt to the player's list of attempts
        pair<char, int> att(col, row);
        to_attack->addAttempt(att);

        // sending the result of the new attempt
        vector<tuple<char, int, char>> new_foreign_attemps = to_attack->getAttempts();
        sendAttempt(new_foreign_attemps.back());
        if (this->status != good)
            return this->status;

        // at the end we unlock and set attack back to false
        turn_locker.unlock();
        this->attack = false;
    }

    return good;
}

void Player::addShip()
{
    // aircraft carrier - col, row, orient
    // sets the attribute status to the return value of checkBytesRec() - which is then handled in the main thread code
    // getting the col
    char col;
    bytes_rec = recv(this->sockfd, &col, 1, MSG_WAITALL);
    status = checkBytesRec();
    if (status != threadvalue::good)
        return;

    // getting the row
    bytes_rec = recv(this->sockfd, &tmp, sizeof(int), MSG_WAITALL);
    status = checkBytesRec();
    if (status != threadvalue::good)
        return;
    int row = ntohl(tmp);

    // getting the orientation
    char orient;
    bytes_rec = recv(this->sockfd, &orient, 1, MSG_WAITALL);
    status = checkBytesRec();
    if (status != threadvalue::good)
        return;

    // creating the ship tuple
    tuple<char, int, char> pos(col, row, orient);
    this->ship_pos.push_back(pos);
}

void Player::closeSocket()
{
    lock_guard<mutex> l(this->player_mutex);
    close(this->sockfd);
}

string Player::getName()
{
    lock_guard<mutex> l(this->player_mutex);
    return this->name;
}

bool Player::getReady()
{
    lock_guard<mutex> l(this->player_mutex);
    return this->ready;
}

bool Player::getAttack()
{
    lock_guard<mutex> l(this->player_mutex);
    return this->attack;
}

void Player::setAttack()
{
    lock_guard<mutex> l(this->player_mutex);
    this->ready = true;
}

bool Player::checkHit(char col_pos, int row_pos, char orient, int length, pair<char, int> attempt)
{
    if (orient == 'H')
    {
        if (attempt.second != row_pos)
            return false;

        if ((attempt.first < col_pos) || (attempt.first > (col_pos + length - 1)))
            return false;
    }
    else if (orient == 'V')
    {
        if (attempt.first != col_pos)
            return false;

        if ((attempt.second > row_pos) || (attempt.second < (row_pos - length + 1)))
            return false;
    }

    return true;
}

void Player::addAttempt(pair<char, int> attempt)
{
    lock_guard<mutex> l(this->player_mutex);

    // checking if the attempt already exists
    for (auto att : attemps)
    {
        if (get<0>(att) == attempt.first && get<1>(att) == attempt.second)
            return;
    }

    vector<int> lengths{5, 4, 3, 2, 2, 1, 1};
    for (int i = 0; i < ship_pos.size(); i++)
    {
        if (checkHit(get<0>(ship_pos[i]), get<1>(ship_pos[i]), get<2>(ship_pos[i]), lengths[i], attempt))
        {
            this->attemps.push_back(make_tuple(attempt.first, attempt.second, 'X'));
            return;
        }
    }

    this->attemps.push_back(make_tuple(attempt.first, attempt.second, 'O'));
    return;
}

vector<tuple<char, int, char>> Player::getAttempts()
{
    lock_guard<mutex> l(this->player_mutex);
    return this->attemps;
}

bool Player::getLost()
{
    lock_guard<mutex> l(this->player_mutex);
    return this->lost;
}

threadvalue Player::checkBytesRec()
{
    switch (this->bytes_rec)
    {
    case -1:
        return threadvalue::localerr;
        break;
    case 0:
        return threadvalue::disconnected;
        break;
    default:
        return threadvalue::good;
    }
}

void Player::sendAttempt(tuple<char, int, char> attempt)
{
    char col = get<0>(attempt);
    if (send(this->sockfd, &col, 1, 0) < 0)
    {
        this->status = localerr;
        cout << "Failed sending the column of the attempt..." << endl;
        return;
    }

    int row = htonl(get<1>(attempt));
    if (send(this->sockfd, &row, sizeof(int), 0) < 0)
    {
        this->status = localerr;
        cout << "Failed sending the row of the attempt..." << endl;
        return;
    }

    char hm = get<2>(attempt);
    if (send(this->sockfd, &hm, 1, 0) < 0)
    {
        this->status = localerr;
        cout << "Failed whether the attempt was a hit or a miss..." << endl;
        return;
    }

    this->status = good;
    return;
}

void Player::sendAllAttempts(Player *p)
{
    // first sends how many attempts there are (HEADER)
    vector<tuple<char, int, char>> foreign_attempts = p->getAttempts();
    int attemps_size = foreign_attempts.size();
    header = htonl(attemps_size);
    if (send(this->sockfd, &header, sizeof(int), 0) < 0)
    {
        this->status = localerr;
        cout << "Failed sending the size of the attempts vector of the player to be attacked..." << endl;
        return;
    }

    for (auto cell : foreign_attempts)
    {
        sendAttempt(cell);
        if (this->status != good)
            return;
    }

    this->status = good;
    return;
}

bool Player::checkIfLost()
{
    // if number of Xs (hits) is equal to number of cells covered by the ships then evberything is sunk and the player lost
    int count_X = 0;
    int num_ship_cells = 5 + 4 + 3 + 4 + 2;
    for (auto attempt : attemps)
    {
        if (get<2>(attempt) == 'X')
            ++count_X;
    }

    if (count_X == num_ship_cells)
        return true;

    return false;
}