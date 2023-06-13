#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "Battleship.h"
#include "Player.h"

using namespace std;

#define MAXROWS 10
#define MAXCOLS 10

class Grid
{
private:
    vector<vector<char>> map{MAXROWS, vector<char>(MAXCOLS, '-')};
    vector<GeneralBattleship *> battleships{7};

public:
    Grid() {}
    Grid(vector<GeneralBattleship *> battleships) : battleships(battleships) {}
    ~Grid()
    {
        for (auto i : battleships)
            delete i;
    }

    void displayGrid()
    {
        cout << " \tA\tB\tC\tD\tE\tF\tG\tH\tI\tJ" << endl;
        for (int i = 0; i < MAXROWS; i++)
        {
            cout << i + 1 << "\t";
            for (int j = 0; j < MAXCOLS + 1; j++)
            {
                if (!onBoat(i, j)) // onBoat takes care of the printing
                    cout << "-\t";
                cout << "\n";
            }
        }
    }

    void addShip(GeneralBattleship *bs)
    {
        this->battleships.push_back(bs);
    }

private:
    bool onBoat(int row, int col)
    {
        bool ans = false;

        for (auto bs : battleships)
        {
            if (bs->orientation == 'H')
            {
                if (row != bs->row)
                {
                    ans = false;
                    break;
                }
                if (col >= bs->col && col <= (bs->col + bs->getLength()) - 1)
                {
                    bs->printSymbol();
                    ans = true;
                    break;
                }
            }
            else
            {
                if (col != bs->col)
                {
                    ans = false;
                    break;
                }
                if (row >= bs->row && row <= (bs->row + bs->getLength()) - 1)

                {
                    ans = true;
                    break;
                }
            }
        }

        return ans;
    }
};