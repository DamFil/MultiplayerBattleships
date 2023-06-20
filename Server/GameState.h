#include <vector>
#include <mutex>
#include <condition_variable>
#include <bits/stdc++.h>
#include "Player.h"

using namespace std;

bool start_game = false;

class GameState
{
private:
    mutex m;
    int num_players;
    int num_spectators;
    vector<Player *> active_players{};

public:
    GameState() : num_players(0), num_spectators(0) {}

    void addPlayer(Player *p)
    {
        lock_guard<mutex> l(this->m);
        active_players.push_back(p);
    }

    void removePlayer(Player *p)
    {
        lock_guard<mutex> l(this->m);
        remove(active_players.begin(), active_players.end(), p);
    }

    void incPlayers(int amnt = 1)
    {
        lock_guard<mutex> l(this->m);
        num_players += amnt;
    }

    void decPlayers()
    {
        lock_guard<mutex> l(this->m);
        --num_players;
    }
};