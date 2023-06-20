#include "Player.h"
#include "GameState.h"

using namespace std;

Player::Player(int sockfd, GameState *gameinfo) : sockfd(sockfd), gameinfo(gameinfo), status(threadvalue::good), turn(false), ready(false), initialized(false), name("") {}

Player::~Player()
{
    lock_guard<mutex> l(this->player_mutex);
    close(this->sockfd);
}

threadvalue Player::getNameAndStart()
{
    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GETTING THE NAME OF THE PLAYER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // we only initialize the name if it was not initialized
    if (name == "")
    {
        // getting the header
        bytes_rec = recv(this->sockfd, &header, sizeof(header), 0);
        int name_len = ntohl(this->header); // converting to server byte ordering

        // reading the actual name
        char buf[name_len];
        bytes_rec = recv(this->sockfd, buf, name_len, MSG_WAITALL); // waiting for all the bytes
        threadvalue res = checkBytesRec();
        if (res != threadvalue::good)
            return res;

        this->name = buf;
    }

    // getting the answer for starting the game
    char sg;
    bytes_rec = recv(this->sockfd, &sg, sizeof(char), 0); // sizeof char == 1 byte
    threadvalue res = checkBytesRec();
    if (res != threadvalue::good)
        return res;

    if (sg == 'Y')
    {
        gameinfo->changeStartGame(true);
    }
    else if (sg == 'Q')
    {
        return threadvalue::disconnected;
    }

    string pre_game = "Starting...";
    tmp = pre_game.length();
    header = htonl(tmp);
    bytes_sent = send(this->sockfd, &header, sizeof(int), 0);

    bytes_sent = send(this->sockfd, pre_game.c_str(), header, 0);

    return threadvalue::good;
}

void Player::playerInitThread()
{
    // Intilizing the ships
    cout << "Getting " << this->name << "'s ship positions..." << endl;
    if (!getAllShips())
        return;

    this->initialized = true;
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

bool Player::getShip()
{
    int col, row;

    // aircraft carrier
    this->bytes_rec = recv(this->sockfd, &this->tmp, sizeof(col), 0);
    if (!checkBytesRec())
        return false;
    col = ntohl(tmp);

    this->bytes_rec = recv(this->sockfd, &this->tmp, sizeof(col), 0);
    if (!checkBytesRec())
        return false;
    row = ntohl(tmp);

    pair<int, int> coords(col, row);

    char orientation;
    this->bytes_rec = recv(this->sockfd, &orientation, sizeof(char), 0);
    if (!checkBytesRec())
        return false;

    pair<pair<int, int>, char> ship(coords, orientation);
    this->ship_pos.push_back(ship);

    return true;
}

bool Player::getAllShips()
{
    if (!getShip()) // aircraft carrier
        return false;
    if (!getShip()) // battleship
        return false;
    if (!getShip()) // cruiser
        return false;
    if (!getShip()) // destroyer
        return false;
    if (!getShip()) // destroyer
        return false;
    if (!getShip()) // submarine
        return false;
    if (!getShip()) // submarine
        return false;

    return true;
};