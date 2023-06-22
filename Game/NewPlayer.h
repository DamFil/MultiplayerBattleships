#pragma once

#include <map>
#include "Grid.h"
#include "Battleship.h"

enum ShipType
{
    A,
    B,
    C,
    D,
    S
};

class NewPlayer
{
public:
    Grid *grid;

    NewPlayer() : grid(new Grid()) {}
    ~NewPlayer()
    {
        delete grid;
    }

    bool newShip(ShipType t, char col, int row, char orientation);
    void showMap();
    void showAttemptsMap();
    bool attack(NewPlayer *player, char col, int row);

private:
    inline bool checkBound(char col, int row, int length, char orientation);

    map<char, int> ctoi =
        {{'A', 0},
         {'B', 1},
         {'C', 2},
         {'D', 3},
         {'E', 4},
         {'F', 5},
         {'G', 6},
         {'H', 7},
         {'I', 8},
         {'J', 9}};
};