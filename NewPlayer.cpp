#include "NewPlayer.h"

inline bool NewPlayer::checkBound(char col, int row, int length, char orientation)
{
    if ((orientation == 'H' && col > ('J' - length + 1)) || (orientation == 'V' && row < length))
    {
        cout << "The ship does not fit on the grid!" << endl;
        exit(EXIT_FAILURE);
    }

    return true;
}

void NewPlayer::newShip(ShipType t, char col, int row, char orientation)
{
    switch (t)
    {
    case A:
    {
        checkBound(col, row, 5, orientation);
        AircraftCarrier *ac = new AircraftCarrier(orientation, row - 1, ctoi[col]);
        grid->addShip(ac);
        break;
    }
    case B:
    {
        checkBound(col, row, 4, orientation);
        Battleship *bs = new Battleship(orientation, row - 1, ctoi[col]);
        grid->addShip(bs);
        break;
    }
    case C:
    {
        checkBound(col, row, 3, orientation);
        Cruiser *cs = new Cruiser(orientation, row - 1, ctoi[col]);
        grid->addShip(cs);
        break;
    }
    case D:
    {
        checkBound(col, row, 2, orientation);
        Destroyer *ds = new Destroyer(orientation, row - 1, ctoi[col]);
        grid->addShip(ds);
        break;
    }
    case S:
    {
        checkBound(col, row, 1, orientation);
        Submarine *sm = new Submarine(orientation, row - 1, ctoi[col]);
        grid->addShip(sm);
        break;
    }
    }

    return;
}

void NewPlayer::showMap()
{
    grid->displayMap();
}

// bool NewPlayer::attack(NewPlayer *p, char col, int row)
//{
// }