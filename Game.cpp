#include "NewPlayer.h"

int main()
{
    NewPlayer *p = new NewPlayer();

    p->newShip(A, 'A', 2, 'H');
    p->newShip(B, 'C', 6, 'V');
    p->newShip(C, 'F', 3, 'H');
    p->newShip(D, 'D', 7, 'V');
    p->newShip(D, 'A', 10, 'H');
    p->newShip(S, 'J', 10, 'V');
    p->newShip(S, 'I', 9, 'V');

    p->showGrid();

    return 0;
}