#include "ConnectionManager.h"

int main(int argc, char *argv[])
{
    // we get the portnum from the command line arguments
    string portnum;

    if (argc != 2)
    {
        cout << "Usage: ./server portnum" << endl;
        portnum = "5001";
    }
    else
        portnum = argv[1];

    // setting up the network connection
    ConnManager *connm = new ConnManager(portnum);
    connm->setupAndListen();
    connm->acceptConnections();

    return 0;
}