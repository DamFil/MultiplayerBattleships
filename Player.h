#pragma once

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
    void attack(Player *player);

private:
    bool parseCell(string cell, char *col, int *row);
    inline bool checkBound(ShipType t, char col, int row, char orientation);
    void gettingShipInfo(ShipType t, char *col, int *row, char *orientation);
    void getShip(ShipType t);
    void getAllShips();
};