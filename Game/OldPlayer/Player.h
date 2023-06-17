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

class Player
{
public:
    Grid *grid;
    string name;

    Player() : grid(new Grid())
    {
        cout << "Enter the name of your character" << endl;
        cin >> name;
    }
    ~Player()
    {
        delete grid;
    }

    void initializeGrid();
    void showGrid();
    void attack(Player *player);

private:
    bool parseCell(string cell, char *col, int *row);
    inline bool isOnBoat(int row, char col);
    inline bool checkBound(ShipType t, char col, int row, char orientation);
    void gettingShipInfo(ShipType t, char *col, int *row, char *orientation);
    void getShip(ShipType t);
    void getAllShips();

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