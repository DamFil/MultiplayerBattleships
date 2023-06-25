#include "../ConnManager.h"
#include "../Client.h"

int main(int argc, char *argv[])
{
    //* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PARSING COMMAND LINE ARGUMENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 1) battleships 2) ip:portnum 3) player's name
    if (argc != 3)
    {
        cout << "Usage: battleships ip:portnum name" << endl;
        return EXIT_SUCCESS;
    }

    string iport = argv[1];
    size_t poscol = iport.find_first_of(':', 6); // 6 because the minimum length ip is 7 chars long (1.1.1.1)
    if (poscol == string::npos)
    {
        cout << "The first argument has to be ip:portnum. For example: 192.168.2.3:5001" << endl;
        return EXIT_SUCCESS;
    }

    string ip = iport.substr(0, poscol);
    string portnum = iport.substr(poscol + 1);
    string name = argv[2];

    cout << "ip: [ " << ip << " ]" << endl;
    cout << "port number: [ " << portnum << " ]" << endl;
    cout << "name: [ " << name << " ]" << endl;

    ConnManager *connm = new ConnManager(ip, portnum);
    if (connm->setupAndConnect() == -1)
    {
        cout << "Problems with setting up the connection..." << endl;
        delete connm;
        return EXIT_FAILURE;
    }

    Client *client = new Client(connm->getSocket(), name);
    clientvalue rtrn = client->initPlayer();
    if (rtrn != good)
    {
        cout << "Error communcating with the client..." << endl;
        connm->closeSocket();
        delete client;
        delete connm;
        return EXIT_FAILURE;
    }

    client->addShipLocal(A, 'A', 1, 'H');
    client->sendShip('A', 1, 'H');

    client->addShipLocal(B, 'E', 3, 'H');
    client->sendShip('E', 3, 'H');

    client->addShipLocal(C, 'C', 9, 'V');
    client->sendShip('C', 9, 'V');

    client->addShipLocal(D, 'A', 10, 'H');
    client->sendShip('A', 10, 'H');

    client->addShipLocal(D, 'H', 6, 'V');
    client->sendShip('H', 6, 'V');

    client->addShipLocal(S, 'J', 10, 'V');
    client->sendShip('J', 10, 'V');

    client->addShipLocal(S, 'I', 9, 'V');
    client->sendShip('I', 9, 'V');

    rtrn = client->attack();
    if (rtrn != good)
    {
        cout << "Error communcating with the client..." << endl;
        connm->closeSocket();
        delete client;
        delete connm;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}