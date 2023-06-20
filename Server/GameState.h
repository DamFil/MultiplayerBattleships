#include <vector>
#include <mutex>
#include <condition_variable>
#include <bits/stdc++.h>
#include "Player.h"

using namespace std;

enum threadvalue
{
    disconnected,
    localerr,
    good
};

class GameState
{
private:
    mutex m;
    int num_players;
    int num_spectators;
    vector<Player *> active_players{};
    bool start_game;

public:
    GameState() : num_players(0), num_spectators(0), start_game(true) {}

    void addPlayer(Player *p)
    {
        lock_guard<mutex> l(this->m);
        active_players.push_back(p);
        ++num_players;
    }

    void removePlayer(Player *p)
    {
        lock_guard<mutex> l(this->m);
        remove(active_players.begin(), active_players.end(), p);
        --num_players;
    }

    // void incPlayers(int amnt = 1)
    //{
    //     lock_guard<mutex> l(this->m);
    //     num_players += amnt;
    // }
    //
    // void decPlayers()
    //{
    //    lock_guard<mutex> l(this->m);
    //    --num_players;
    //}

    int getNumPlayers()
    {
        lock_guard<mutex> l(this->m);
        return this->num_players;
    }

    bool getStartGame()
    {
        lock_guard<mutex> l(this->m);
        return this->start_game;
    }

    void changeStartGame(bool playerchoice)
    {
        lock_guard<mutex> l(this->m);
        this->start_game = true;
        this->start_game &= playerchoice;
    }

    vector<Player *> getPlayers()
    {
        lock_guard<mutex> l(this->m);
        return this->active_players;
    }
};