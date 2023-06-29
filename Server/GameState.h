#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include "Player.h"

using namespace std;

class GameState
{
private:
    mutex m;
    vector<Player *> active_players{};
    vector<int> spectators{};
    bool stop_connect; // notifies the main thread to stop accepting further connections for players

public:
    mutex temp_lock;
    condition_variable turns_start;
    GameState() : stop_connect(false) {}

    void addPlayer(Player *p)
    {
        lock_guard<mutex> l(this->m);
        active_players.push_back(p);
    }

    void removePlayer(Player *p)
    {
        lock_guard<mutex> l(this->m);

        if (p == nullptr)
            return;

        int i;
        bool found = false;
        for (i = 0; i < active_players.size(); i++)
        {
            if (active_players.at(i) == p)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return;

        p->closeSocket();
        delete p;
        active_players.erase(active_players.begin() + i);
    }

    void removePlayer(int player_pos)
    {
        lock_guard<mutex> l(this->m);
        Player *p;
        try
        {
            p = this->active_players.at(player_pos);
        }
        catch (std::out_of_range)
        {
            return;
        }

        p->closeSocket();
        active_players.erase(active_players.begin() + player_pos);
        delete p;
    }

    Player *getPlayer(int i)
    {
        lock_guard<mutex> l(this->m);
        try
        {
            return this->active_players.at(i);
        }
        catch (std::out_of_range)
        {
            return nullptr;
        }
    }

    Player *getPlayer(string name)
    {
        lock_guard<mutex> l(this->m);
        Player *player;
        bool found = false;
        for (auto p : active_players)
        {
            if (p->getName() == name)
            {
                found = true;
                player = p;
                break;
            }
        }

        if (found == false)
            return nullptr;

        return player;
    }

    int getNumPlayers()
    {
        lock_guard<mutex> l(this->m);
        return this->active_players.size();
    }

    void addSpectator(int sockfd)
    {
        lock_guard<mutex> l(this->m);
        this->spectators.push_back(sockfd);
    }

    void removeSpectator(int sockfd)
    {
        lock_guard<mutex> l(this->m);
        bool found = false;
        int i;
        for (i = 0; i < spectators.size(); i++)
        {
            if (spectators[i] == sockfd)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return;

        this->spectators.erase(spectators.end() + i);
        return;
    }

    int getSpectator(int i)
    {
        lock_guard<mutex> l(this->m);
        try
        {
            return this->spectators.at(i);
        }
        catch (const std::out_of_range)
        {
            return -1;
        }
    }

    int getNumSpectators()
    {
        lock_guard<mutex> l(this->m);
        return this->spectators.size();
    }

    bool getStartGame()
    {
        lock_guard<mutex> l(this->m);
        bool ans = true;
        for (int i = 0; i < active_players.size(); i++)
        {
            ans &= active_players.at(i)->getReady();
        }

        if (ans && this->active_players.size() >= 2)
        {
            turns_start.notify_one();
            this->stop_connect = true;
        }

        return stop_connect;
    }

    bool getStopConnect()
    {
        lock_guard<mutex> l(this->m);
        return this->stop_connect;
    }

    string getNames(Player *player)
    {
        lock_guard<mutex> l(this->m);
        string names = "";
        for (auto p : active_players)
        {
            if (p == player)
                continue;
            names.append(p->getName() + " ");
        }

        return names;
    }
};