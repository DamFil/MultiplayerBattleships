#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "Battleship.h"
#include "NewPlayer.h"

using namespace std;

#define MAXROWS 10
#define MAXCOLS 10

enum Attempt
{
    hit,
    miss,
    none
};

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

    void updateMap()
    {
        // we update the map only for the last element because we call this function after we add another ship
        if (!battleships.empty())
        {
            auto bs = battleships.back();
            if (bs->orientation == 'H')
            {
                for (int i = bs->col; i < bs->col + bs->length; i++)
                {
                    map.at(bs->row).at(i) = bs->getSymbol();
                }
            }
            else if (bs->orientation == 'V')
            {
                for (int i = bs->row; i > bs->row - bs->length; i--)
                {
                    map.at(i).at(bs->col) = bs->getSymbol();
                }
            }
        }
    }

    bool addShip(GeneralBattleship *bs)
    {
        // checks for boats that would collide - returns false if the new boat collides with an already added boat
        for (auto bts : battleships)
        {
            if (bs->orientation == 'H')
            {
                if (bts->orientation == 'H' && bts->row == bs->row)
                {
                    for (int i = 0; i < bs->getLength(); i++)
                    {
                        if (bs->col + i >= bts->col && bs->col + i <= (bts->col + bts->getLength() - 1))
                        {
                            cout << "The added ship collides with an already placed ship!" << endl;
                            return false;
                        }
                    }
                }
                else if (bts->orientation == 'V')
                {
                    if ((bs->row <= bts->row) && (bs->row >= (bts->row - bts->getLength() + 1)) && ((bts->col >= bs->col) && (bts->col <= (bs->col + bs->getLength() - 1))))
                    {
                        cout << "The added ship collides with an already placed ship!" << endl;
                        return false;
                    }
                }
                else
                    continue;
            }
            else if (bs->orientation == 'V')
            {
                if (bts->orientation == 'V' && bts->col == bs->col)
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
                else if ((bts->row <= bs->row) && (bts->row >= (bs->row - bs->getLength() + 1)) && ((bs->col >= bts->col) && (bs->col <= (bts->col + bts->getLength() - 1))))
                {
                    cout << "The added ship collides with an already placed ship!" << endl;
                    return false;
                }
                else
                    continue;
            }
        }

        this->battleships.push_back(bs);
        cout << "The ship has been added successfully!" << endl;
        updateMap();
        return true;
    }

    void addAttempts(int row, int col, char hm)
    {
        map.at(row).at(col) = hm;
        // TODO: Make the map display F for fully sunk ships
    }

    void printMap(vector<vector<char>> foreign_map)
    {
        string alphabet = "   A  B  C  D  E  F  G  H  I  J";
        cout << alphabet << "\n"
             << endl;
        for (int i = 0; i < MAXROWS; i++)
        {
            cout << i + 1 << "  ";
            for (int j = 0; j < MAXCOLS; j++)
            {
                cout << foreign_map.at(i).at(j) << "  ";
            }
            cout << "\n\n";
        }
    }

    void printMyMap()
    {
        cout << "My map:\n"
             << endl;
        printMap(this->map);
    }
};