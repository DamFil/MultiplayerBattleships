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
    bytes_sent = send(this->sockfd, &st, 1, 0);
    threadvalue res = checkBytesRec();
    if (res != threadvalue::good)
        return res;

    // getting the header
    bytes_rec = recv(this->sockfd, &header, sizeof(header), 0);
    int name_len = ntohl(this->header); // converting to server byte ordering

    // reading the actual name
    char buf[name_len];
    bytes_rec = recv(this->sockfd, buf, name_len, MSG_WAITALL); // waiting for all the bytes
    res = checkBytesRec();
    if (res != threadvalue::good)
        return res;

    this->name = buf;

    // getting the answer for starting the game
    char sg;
    bytes_rec = recv(this->sockfd, &sg, sizeof(char), MSG_WAITALL); // sizeof char == 1 byte
    res = checkBytesRec();
    if (res != threadvalue::good)
        return res;

    // only Y can be sent since the client disconnects on Q - call to recv returns 0

    // Initilizing the ships
    for (int i = 0; i < MAX_SHIPS; i++)
    {
        addShip();
        if (this->status != good)
            return this->status;
    }
    this->ready = true;

    // we have to wait for all the players to finish their initialization
    while (!gameinfo->getStartGame())
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void Player::addShip()
{
    // aircraft carrier - col, row, orient
    // sets the attribute status to the return value of checkBytesRec() - which is then handled in the main thread code
    bytes_rec = recv(this->sockfd, &tmp, sizeof(int), MSG_WAITALL);
    status = checkBytesRec();
    if (status != threadvalue::good)
        return;
    int col = ntohl(tmp);

    bytes_rec = recv(this->sockfd, &tmp, sizeof(int), MSG_WAITALL);
    status = checkBytesRec();
    if (status != threadvalue::good)
        return;
    int row = ntohl(tmp);

    char orient;
    bytes_rec = recv(this->sockfd, &orient, 1, MSG_WAITALL);
    status = checkBytesRec();
    if (status != threadvalue::good)
        return;

    tuple<int, int, char> pos(col, row, orient);
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