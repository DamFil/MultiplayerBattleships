#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "Battleship.h"
#include "NewPlayer.h"

using namespace std;

#define MAXROWS 10
#define MAXCOLS 10

class Grid
{
private:
    vector<vector<char>> map{MAXROWS, vector<char>(MAXCOLS, '-')};
    vector<GeneralBattleship *> battleships{};

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
        cout << " \tA\tB\tC\tD\tE\tF\tG\tH\tI\tJ\n\n"
             << endl;
        for (int i = 0; i < MAXROWS; i++)
        {
            cout << i + 1 << "\t";
            for (int j = 0; j < MAXCOLS; j++)
            {
                GeneralBattleship *bs = isOnBoat(i, j);
                if (bs != nullptr)
                    bs->printSymbol();
                else
                    cout << "-\t";
            }
            cout << "\n\n\n";
        }
    }

    bool addShip(GeneralBattleship *bs)
    {
        // checks for boats that would collide - returns false if the new boat would collide with an already added boat
        for (auto bts : battleships)
        {
            if (bs->orientation == 'H')
            {
                if (bs->row == bts->row)
                {
                    for (int i = 0; i < bs->getLength(); i++)
                    {
                        if (bs->col + i >= bts->col && bs->col + i <= (bts->col + bts->getLength() - 1))
                        {
                            cout << "The added ship collides with an already exisiting one!" << endl;
                            return false;
                        }
                    }
                }
                else
                    continue;
            }
            else if (bs->orientation == 'V')
            {
                if (bs->col == bts->col)
                {
                    for (int i = 0; i < bs->getLength(); i++)
                    {
                        if (bs->row + i <= bts->row && bs->row + i >= (bts->row + bts->getLength() + 1))
                        {
                            cout << "The added ship collides with an already existing one!" << endl;
                            return false;
                        }
                    }
                }
                else
                    continue;
            }
        }

        this->battleships.push_back(bs);
        cout << "Teh ship has been added successfully!" << endl;
        return true;
    }

    GeneralBattleship *isOnBoat(int row, int col)
    {
        for (auto bs : battleships)
        {
            if (bs->orientation == 'H')
            {
                if (row == bs->row && (col >= bs->col && col <= (bs->col + bs->getLength() - 1)))
                    return bs;
                continue;
            }
            else if (bs->orientation == 'V')
            {
                if (col == bs->col && (row <= bs->row && row >= (bs->row - bs->getLength() + 1)))
                    return bs;
                continue;
            }
        }

        return nullptr;
    }
};