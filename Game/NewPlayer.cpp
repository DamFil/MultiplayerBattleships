#include "NewPlayer.h"

inline bool NewPlayer::checkBound(char col, int row, int length, char orientation)
{
    if ((orientation == 'H' && col > ('J' - length + 1)) || (orientation == 'V' && row < length))
    {
        cout << "The ship does not fit on the grid!" << endl;
        return false;
    }

    return true;
}

bool NewPlayer::newShip(ShipType t, char col, int row, char orientation)
{
    switch (t)
    {
    case A:
    {
        if (!checkBound(col, row, 5, orientation))
            return false;
        AircraftCarrier *ac = new AircraftCarrier(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(ac))
        {
            delete ac;
            return false;
        }
        break;
    }
    case B:
    {
        if (!checkBound(col, row, 4, orientation))
            return false;
        Battleship *bs = new Battleship(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(bs))
        {
            delete bs;
            return false;
        }
        break;
    }
    case C:
    {
        if (!checkBound(col, row, 3, orientation))
            return false;
        Cruiser *cs = new Cruiser(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(cs))
        {
            delete cs;
            return false;
        }
        break;
    }
    case D:
    {
        if (!checkBound(col, row, 2, orientation))
            return false;
        Destroyer *ds = new Destroyer(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(ds))
        {
            delete ds;
            return false;
        }
        break;
    }
    case S:
    {
        if (!checkBound(col, row, 1, orientation))
            return false;
        Submarine *sm = new Submarine(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(sm))
        {
            delete sm;
            return false;
        }
        break;
    }
    }

    return true;
}

void NewPlayer::showMap(vector<tuple<char, int, char>> foreign_attempts)
{
    vector<vector<char>> foreign_map{MAXROWS, vector<char>(MAXCOLS, '-')};
    for (auto att : foreign_attempts)
    {
        int row = get<1>(att) - 1;
        int col = ctoi[get<0>(att)];
        foreign_map.at(row).at(col) = get<2>(att);
    }
    grid->printMyMapAndAttack(foreign_map);
}

void NewPlayer::showMap()
{
    grid->displayMap();
}

void NewPlayer::addAttempt(tuple<char, int, char> att)
{
    int col = ctoi[get<0>(att)];
    int row = get<1>(att) - 1;
    char hm = get<2>(att);
    grid->addAttempts(col, row, hm);
}