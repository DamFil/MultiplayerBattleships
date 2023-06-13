#include "Player.h"

/*
TODO: WHEN YOU BOUNDS CHECK FOR SHIPS MAKE SURE THAT THE NEWLY ADDED SHIP IS NOT COLLIDING WITH ANOTHER ONE ON THE GRID

*/

bool Player::parseCell(string cell, char *col, int *row)
{
    if (cell.length() < 2 || cell.length() > 3)
        return false;
    if (!(isalpha(cell.at(0))) && (cell.at(0) < 'A' || cell.at(0) > 'J'))
        return false;
    if (!isdigit(cell.at(1)))
        return false;
    if (cell.length() == 3 && !isdigit(cell.at(2)))
        return false;

    *col = cell.at(0);
    *row = stoi(cell.substr(1));

    return true;
}

// checks that the ship can be placed on the grid by checking the bounds
inline bool Player::checkBound(ShipType t, char col, int row, char orientation)
{
    bool ans = true;

    switch (t)
    {
    case A:
        if (orientation == 'H')
        {
            if (col > 'F')
                ans = false;
            else
                ans = true;
        }
        else
        {
            if (row < 5)
                ans = false;
            else
                ans = true;
        }
        break;
    case B:
        if (orientation == 'H')
        {
            if (col > 'G')
                ans = false;
            else
                ans = true;
        }
        else
        {
            if (row < 4)
                ans = false;
            else
                ans = true;
        }
        break;
    case C:
        if (orientation == 'H')
        {
            if (col > 'H')
                ans = false;
            else
                ans = true;
        }
        else
        {
            if (row < 3)
                ans = false;
            else
                ans = true;
        }
        break;
    case D:
        if (orientation == 'H')
        {
            if (col > 'I') // i.e. col == J
                ans = false;
            else
                ans = true;
        }
        else
        {
            if (row < 2) // i.e. row == 1
                ans = false;
            else
                ans = true;
        }
        break;
    case S:
        ans = true; // regardless of the orientation it is fine since it occupies only 1 cell
        break;
    }

    return ans;
}

// asks all the details for the ship and calls the checkBound function
void Player::gettingShipInfo(ShipType t, char *col, int *row, char *orientation)
{
    string cell;
    // keeps track of the iteration
    int i = 0;

    do
    {
        ++i;
        if (i > 1)
            cout << "The ship does not fit on the grid! Pick another location: " << endl;

        cin >> cell;
        while (!parseCell(cell, col, row))
        {
            cout << "Answer with column character (A-J) followed by row number (1-10)" << endl;
            cin >> cell;
        }

        // getting ship orientation
        cout << "Please enter the orientation (H/V)" << endl;
        cin >> *orientation;

        while (*orientation != 'H' && *orientation != 'V')
        {
            cout << "Answer with H or V" << endl;
            cin >> *orientation;
        }
    } while (!checkBound(t, *col, *row, *orientation));
}

// Initializes the right ship
void Player::getShip(ShipType t)
{
    char col;
    int row;
    char orientation;

    // we are intializing with row - 1 because the inputted row will be from 1 - 10 and we need from 0 - 9
    switch (t)
    {
    case ShipType::A:
    {
        cout << "Please enter the starting cell for the Aircraft Carrier: " << endl;
        gettingShipInfo(t, &col, &row, &orientation);
        AircraftCarrier *ac = new AircraftCarrier(orientation, row - 1, itoc.at(col));
        grid->addShip(ac);
        break;
    }
    case ShipType::B:
    {
        cout << "Please enter the starting cell for the Battleship: " << endl;
        gettingShipInfo(t, &col, &row, &orientation);
        Battleship *bs = new Battleship(orientation, row - 1, itoc.at(col));
        grid->addShip(bs);
        break;
    }
    case ShipType::C:
    {
        cout << "Please enter the starting cell for the Cruiser: " << endl;
        gettingShipInfo(t, &col, &row, &orientation);
        Cruiser *cs = new Cruiser(orientation, row - 1, itoc.at(col));
        grid->addShip(cs);
        break;
    }
    case ShipType::D:
    {
        cout << "Please enter the starting cell for the Destroyer: " << endl;
        gettingShipInfo(t, &col, &row, &orientation);
        Destroyer *ds = new Destroyer(orientation, row - 1, itoc.at(col));
        grid->addShip(ds);
        break;
    }
    case ShipType::S:
    {
        cout << "Please enter the starting cell for the Submarine: " << endl;
        gettingShipInfo(t, &col, &row, &orientation);
        Submarine *sm = new Submarine(orientation, row - 1, itoc.at(col));
        grid->addShip(sm);
        break;
    }
    }
}

void Player::getAllShips()
{
    getShip(A);
    getShip(B);
    getShip(C);
    getShip(D);
    getShip(D);
    getShip(S);
    getShip(S);
}

void Player::initializeGrid()
{
    char ans;
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    cout << this->name << ":" << endl;
    cout << "Start Game (Y/Q)?" << endl;
    cin >> ans;
    while (ans != 'Y' && ans != 'Q')
    {
        cout << "Answer with Y(yes) or with Q(quit)" << endl;
        cin >> ans;
    }

    if (ans == 'Q')
        exit(EXIT_SUCCESS);

    getAllShips();
}

// wrapper for Grid object's display grid function
void Player::showGrid()
{
    grid->displayGrid();
}