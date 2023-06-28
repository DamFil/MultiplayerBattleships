#include "Spectator.h"

Spectator::Spectator(GameState *gameinfo) : gameinfo(gameinfo), ships_sent(false) {}
Spectator::~Spectator() { delete gameinfo; }

threadvalue Spectator::spectateGame()
{
    int num_spectators = gameinfo->getNumSpectators();
    int num_players = gameinfo->getNumPlayers();
    // every turn sends the ship positions and the attempts
    for (int i = 0; i < num_spectators; i++)
    {
        int sockfd = gameinfo->getSpectator(i);
        if (num_players <= 1)
        {
            sendMessage(sockfd, 'F'); // notifies the spectators that the game is finished
            if (this->status != good)
            {
                gameinfo->removeSpectator(sockfd);
            }
            continue;
        }

        sendMessage(sockfd, 'P');
        if (this->status != good)
        {
            gameinfo->removeSpectator(sockfd);
            continue;
        }
        // send the spectator number of players
        sendInt(sockfd, num_players);
        if (this->status != good)
        {
            gameinfo->removeSpectator(sockfd);
            continue;
        }
        for (int j = 0; j < num_players; j++)
        {
            Player *p = gameinfo->getPlayer(j);

            // sending the ship positions
            vector<tuple<char, int, char>> ship_pos = p->getShips();
            sendShipTuples(sockfd, ship_pos);
            if (this->status != good)
            {
                gameinfo->removeSpectator(sockfd);
                continue;
            }

            // sending the attempts
            sendAllAttempts(sockfd, p);
            if (this->status != good)
            {
                gameinfo->removeSpectator(sockfd);
                continue;
            }
        }
    }
    return good;
}

threadvalue Spectator::sendShips(int num_spectators, int num_players)
{
    for (int i = 0; i < num_spectators; i++)
    {
        int sockfd = gameinfo->getSpectator(i);
        // sending the number of players first
        sendInt(sockfd, num_players);
        if (this->status != good)
        {
            gameinfo->removeSpectator(sockfd);
            continue;
        }
        // sending the actual ship positions of each of the players
        for (int j = 0; i < num_players; j++)
        {
            Player *p = gameinfo->getPlayer(j);
            if (p == nullptr)
                continue;
            vector<tuple<char, int, char>> player_ships = p->getShips();
            sendShipTuples(sockfd, player_ships);
            if (this->status != good)
            {
                gameinfo->removeSpectator(sockfd);
                continue;
            }
        }
    }

    this->ships_sent = true;
    return good;
}

void Spectator::checkBytesRec(int bytes_rec)
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

void Spectator::sendMessage(int sockfd, string msg)
{
    // sending msg length
    sendInt(sockfd, msg.length());
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

void Spectator::sendMessage(int sockfd, char msg)
{
    if (send(sockfd, &msg, 1, 0) < 0)
    {
        this->status = localerr;
        return;
    }

    this->status = good;
    return;
}

string Spectator::recvMessage(int sockfd)
{
    // receving the length
    int header = recvInt(sockfd);
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

char Spectator::recvChar(int sockfd)
{
    char received;
    int bytes_rec = recv(sockfd, &received, 1, MSG_WAITALL);
    checkBytesRec(bytes_rec);
    if (this->status != good)
        return ERROR_CHAR;

    this->status = good;
    return received;
}

void Spectator::sendInt(int sockfd, int x)
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

int Spectator::recvInt(int sockfd)
{
    int received;
    int bytes_rec = recv(sockfd, &received, sizeof(int), 0);
    checkBytesRec(bytes_rec);
    if (this->status != good)
        return ERROR_INT;

    return ntohl(received);
}

void Spectator::sendAttempt(int sockfd, tuple<char, int, char> attempt)
{
    char col = get<0>(attempt);
    sendMessage(sockfd, col);
    if (this->status != good)
        return;

    int row = get<1>(attempt);
    sendInt(sockfd, row);
    if (this->status != good)
        return;

    char hm = get<2>(attempt);
    sendMessage(sockfd, hm);
    if (this->status != good)
        return;

    this->status = good;
    return;
}

void Spectator::sendAllAttempts(int sockfd, Player *p)
{
    // first sends the size of the attempts vector
    vector<tuple<char, int, char>> foreign_attempts = p->getAttempts();
    int attemps_size = foreign_attempts.size();
    sendInt(sockfd, attemps_size);
    if (this->status != good)
        return;

    // then send the actual attempts
    for (auto cell : foreign_attempts)
    {
        sendAttempt(sockfd, cell);
        if (this->status != good)
            return;
    }

    this->status = good;
    return;
}

void Spectator::sendShipTuples(int sockfd, vector<tuple<char, int, char>> ships)
{
    // Client knows he needs to recv 7 ships (no more no less)
    for (auto ship : ships)
    {
        sendAttempt(sockfd, ship);
        if (this->status != good)
            return;
    }

    this->status = good;
    return;
}