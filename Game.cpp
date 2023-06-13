#include "Player.h"

int main()
{
    Player *p = new Player();
    p->initializeGrid();
    p->showGrid();

    return 0;
}