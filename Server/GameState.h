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
    int num_players;
    int num_spectators;
    vector<Player *> active_players{};
    bool stop_connect; // notifies the main thread to stop accepting further connections for players

public:
    mutex turn_lock;
    condition_variable turn_notifier;
    GameState() : num_players(0), num_spectators(0), stop_connect(false) {}

    void addPlayer(Player *p)
    {
        lock_guard<mutex> l(this->m);
        active_players.push_back(p);
        ++num_players;
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
        --num_players;
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
        --num_players;
    }

    void incPlayers()
    {
        lock_guard<mutex> l(this->m);
        ++num_players;
    }

    void decPlayers()
    {
        lock_guard<mutex> l(this->m);
        --num_players;
    }

    int getNumPlayers()
    {
        lock_guard<mutex> l(this->m);
        return this->num_players;
    }

    bool getStartGame()
    {
        lock_guard<mutex> l(this->m);
        bool ans = true;
        for (int i = 0; i < active_players.size(); i++)
        {
            ans &= active_players[i]->getReady();
        }

        return ans;
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

    vector<Player *> getPlayers()
    {
        lock_guard<mutex> l(this->m);
        return this->active_players;
    }

    void setStopConnect(bool value)
    {
        lock_guard<mutex> l(this->m);
        this->stop_connect = value;
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