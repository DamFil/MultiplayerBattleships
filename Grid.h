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
        for (auto bts : battleships)
        {
            if (bs->orientation == 'H')
            {
                if (bs->row == bts->row)
                {
                    for (int i = 0; i < bs->getLength(); i++)
                    {
                        if (bs->col + i == bts->col)
                            return false;
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
                        if (bs->row + i == bts->row)
                            return false;
                    }
                }
                else
                    continue;
            }
        }

        this->battleships.push_back(bs);
        return true;
    }

    GeneralBattleship *isOnBoat(int row, int col)
    {
        for (auto bs : battleships)
        {
            if (row == bs->row && col == bs->col)
                return bs;
            else
                continue;
        }

        return nullptr;
    }

    /*
        GeneralBattleship *onBoat(int row, int col)
        {
            GeneralBattleship *ans = nullptr;

            for (auto bs : battleships)
            {
                if (bs->orientation == 'H')
                {
                    if (row != bs->row)
                    {
                        ans = nullptr;
                        continue;
                    }
                    if (col >= bs->col && col <= (bs->col + bs->getLength() - 1))
                    {
                        ans = bs;
                        break;
                    }
                }
                else
                {
                    if (col != bs->col)
                    {
                        ans = bs;
                        continue;
                    }
                    if (row <= bs->row && row >= (bs->row - bs->getLength() + 1))
                    {
                        ans = bs;
                        break;
                    }
                }
            }

            return ans;
        }
    */
};