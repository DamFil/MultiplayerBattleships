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

threadvalue Player::initPlayer()
{
    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GETTING THE NAME AND GAME START CONFIRMATION OF THE PLAYER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // client waits for this signal to start sending info
    sendMessage('P');
    if (this->status != good)
    {
        cout << "Failure sending thread start signal to client..." << endl;
        return this->status;
    }

    // getting the name
    this->name = recvMessage();
    if (this->status != good)
        return this->status;

    // getting the answer for starting the game
    char sg = recvChar();
    if (this->status != good)
        return this->status;

    // just making sure that we are getting what is expected
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

    // it is important to call this function in order to update the stopConnect value in gameinfo
    gameinfo->getStartGame();
    return good;
}

void Player::addShip()
{
    // aircraft carrier - col, row, orient
    // sets the attribute "status" to the return value of checkBytesRec() - which is then handled in the main thread code

    // getting the col
    char col = recvChar();
    if (this->status != good)
        return;

    // getting the row
    int row = recvInt();
    if (this->status != good)
        return;

    // getting the orientation
    char orient = recvChar();
    if (this->status != good)
        return;

    // creating the ship tuple
    auto pos = make_tuple(col, row, orient);
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

void Player::sendAttempt(tuple<char, int, char> attempt)
{
    char col = get<0>(attempt);
    sendMessage(col);
    if (this->status != good)
        return;

    int row = get<1>(attempt);
    sendInt(row);
    if (this->status != good)
        return;

    char hm = get<2>(attempt);
    sendMessage(hm);
    if (this->status != good)
        return;

    this->status = good;
    return;
}

void Player::sendAllAttempts(Player *p)
{
    // first sends the size of the attempts vector
    vector<tuple<char, int, char>> foreign_attempts = p->getAttempts();
    int attemps_size = foreign_attempts.size();
    sendInt(attemps_size);
    if (this->status != good)
        return;

    // then send the actual attempts
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

threadvalue Player::startAttack()
{
    char sa = 'A';
    // first checks if it lost from the previous players attacks
    if (checkIfLost())
    {
        sendMessage('L');
        if (this->status != good)
            return this->status;
        this->lost = true;
        return lose;
    }
    else if (gameinfo->getNumPlayers() == 1) // this means you have won
    {
        sendMessage('W');
        if (this->status != good)
            return this->status;
        return win;
    }

    sendMessage('A');
    if (this->status != good)
        return this->status;

    //  Sending the attempts this player accumulated over the last turn
    // TODO: You should change this so it just sends the new attempts rather than the whole thing again

    // I first send my attempts
    sendAllAttempts(this);
    if (this->status != good)
        return this->status;

    // send the number of opponents first (number of players minus myself)
    sendInt(gameinfo->getNumPlayers() - 1);
    if (this->status != good)
    {
        cout << "Failed sending the number of opponents..." << endl;
        return this->status;
    }

    for (int i = 0; i < gameinfo->getNumPlayers(); i++)
    {
        //* I send the player's name and then I send his attemtps
        Player *p = gameinfo->getPlayer(i);
        if (p == this)
            continue;

        // send name
        sendMessage(p->getName());
        if (this->status != good)
        {
            cout << "Failed sending the name of the opponent" << endl;
            return this->status;
        }

        sendAllAttempts(p);
        if (this->status != good)
            return this->status;
    }

    // we receive the name of the opponent that we will be attacking and find the corresponding player object
    string to_be_attacked_name = recvMessage();
    Player *to_attack = gameinfo->getPlayer(to_be_attacked_name);

    //* now we need to receive the cell that we are bombing - col row format
    // receiving the column
    char col = recvChar();
    if (this->status != good)
        return this->status;

    // receiving the row
    int row = recvInt();
    if (this->status != good)
        return this->status;

    // we need to add that attempt to the player's list of attempts
    auto att = make_pair(col, row);
    to_attack->addAttempt(att);

    // sending the result of the new attempt
    tuple<char, int, char> last_attempt = to_attack->getAttempts().back();
    sendMessage(get<2>(last_attempt));
    if (this->status != good)
        return this->status;

    return good;
}

void Player::checkBytesRec(int bytes_rec)
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

void Player::sendMessage(string msg)
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

void Player::sendMessage(char msg)
{
    if (send(sockfd, &msg, 1, 0) < 0)
    {
        this->status = localerr;
        return;
    }

    this->status = good;
    return;
}

string Player::recvMessage()
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

char Player::recvChar()
{
    char received;
    int bytes_rec = recv(sockfd, &received, 1, MSG_WAITALL);
    checkBytesRec(bytes_rec);
    if (this->status != good)
        return ERROR_CHAR;

    this->status = good;
    return received;
}

void Player::sendInt(int x)
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

int Player::recvInt()
{
    int received;
    int bytes_rec = recv(sockfd, &received, sizeof(int), 0);
    checkBytesRec(bytes_rec);
    if (this->status != good)
        return ERROR_INT;

    return ntohl(received);
}
