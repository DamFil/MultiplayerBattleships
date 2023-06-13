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
    string name;

    NewPlayer() : grid(new Grid())
    {
        cout << "Enter the name of your character" << endl;
        cin >> name;
    }
    ~NewPlayer()
    {
        delete grid;
    }

    void newShip(ShipType t, char col, int row, char orientation);
    void showGrid();
    void attack(NewPlayer *player);

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