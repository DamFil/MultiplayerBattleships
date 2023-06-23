#include "Player.h"
#include "GameState.h"

using namespace std;

Player::Player(int sockfd, GameState *gameinfo) : sockfd(sockfd), gameinfo(gameinfo),
                                                  status(threadvalue::good), name(""), ready(false) {}

Player::~Player()
{
    lock_guard<mutex> l(this->player_mutex);
    close(this->sockfd);
}

threadvalue Player::getNameAndStart()
{
    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GETTING THE NAME AND GAME START CONFIRMATION OF THE PLAYER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // client waits for this signal to start sending info
    char st = 's';
    bytes_sent = send(this->sockfd, &st, 1, 0); //! - THIS CAUSES A PROBLEM
    if (bytes_sent < 0)
        return localerr;

    // getting the header
    bytes_rec = recv(this->sockfd, &header, sizeof(header), 0);
    int name_len = ntohl(this->header); // converting to server byte ordering

    // reading the actual name
    char buf[name_len];
    bytes_rec = recv(this->sockfd, buf, name_len, MSG_WAITALL); // waiting for all the bytes
    threadvalue res = checkBytesRec();
    if (res != threadvalue::good)
        return res;

    // have to do this because temp_name might contain some unwanted characters
    string temp_name = buf;
    this->name = temp_name.substr(0, name_len);

    // getting the answer for starting the game
    char sg;
    bytes_rec = recv(this->sockfd, &sg, sizeof(char), MSG_WAITALL); // sizeof char == 1 byte
    res = checkBytesRec();
    if (res != threadvalue::good)
        return res;

    if (sg != 'Y')
        return localerr;
    // only Y can be sent since the client disconnects on Q - call to recv returns 0

    //! Initilizing the ships - problem here
    for (int i = 0; i < MAX_SHIPS; i++)
    {
        addShip();
        if (this->status != good)
            return this->status;
    }
    this->ready = true;

    // we have to wait for all the players to finish their initialization (there must be 2 or more players)
    while (gameinfo->getNumPlayers() >= 2 && !gameinfo->getStartGame())
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    char sa = 'A';
    bytes_sent = send(sockfd, &sa, 1, 0);
    if (bytes_sent < 0)
        return localerr;

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