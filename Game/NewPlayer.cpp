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
            return false;
        break;
    }
    case B:
    {
        if (!checkBound(col, row, 4, orientation))
            return false;
        Battleship *bs = new Battleship(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(bs))
            return false;
        break;
    }
    case C:
    {
        if (!checkBound(col, row, 3, orientation))
            return false;
        Cruiser *cs = new Cruiser(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(cs))
            return false;
        break;
    }
    case D:
    {
        if (!checkBound(col, row, 2, orientation))
            return false;
        Destroyer *ds = new Destroyer(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(ds))
            return false;
        break;
    }
    case S:
    {
        if (!checkBound(col, row, 1, orientation))
            return false;
        Submarine *sm = new Submarine(orientation, row - 1, ctoi[col]);
        if (!grid->addShip(sm))
            return false;
        break;
    }
    }

    return true;
}

void NewPlayer::showMap()
{
    grid->displayMap();
}

void NewPlayer::showAttemptsMap()
{
    grid->displayAttempsMap();
}

bool NewPlayer::attack(NewPlayer *p, char col, int row)
{
    if ((col >= 'A' && col <= 'J') && (row >= 1 && row <= 10))
    {
        p->grid->updateAttemptsMap(row - 1, ctoi[col]);
        return true;
    }

    return false;
}