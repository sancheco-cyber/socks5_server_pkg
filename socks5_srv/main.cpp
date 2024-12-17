#include "socks_server.h"
#include <string>

int main(int argc, char * argv[])
{

    int err = 0;
    int srv_life = 5;  /* Durée de vie par défaut */

    /* Le premier argument correspond à la durée de vie du serveur en min */
    if(argc == 2)
    {
        srv_life = std::stoi(argv[1]);
    }

    SocksServer my_server(srv_life);

    /* Lancement du serveur */
    err = my_server.start();

    return err;
}
