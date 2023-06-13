#include "NewPlayer.h"

int main()
{
    NewPlayer *p = new NewPlayer();

    // p->newShip(A, 'A', 2, 'H');
    // p->newShip(B, 'C', 6, 'V');
    // p->newShip(C, 'F', 3, 'H');
    // p->newShip(D, 'D', 7, 'V');
    // p->newShip(D, 'A', 10, 'H');
    // p->newShip(S, 'J', 10, 'V');
    // p->newShip(S, 'I', 9, 'V');

    // p->newShip(A, 'F', 1, 'H');
    // p->newShip(B, 'C', 1, 'H');
    // p->newShip(C, 'H', 4, 'H');
    // p->newShip(D, 'A', 2, 'V');
    // p->newShip(D, 'C', 6, 'H');
    // p->newShip(S, 'F', 7, 'V');
    // p->newShip(S, 'C', 5, 'V');

    p->showGrid();

    return 0;
}