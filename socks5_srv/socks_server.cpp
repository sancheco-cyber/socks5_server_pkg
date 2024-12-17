#include "socks_server.h"
#include "socks_client.h"
#include <unistd.h>
#include <thread>

#include <iostream>


SocksServer::SocksServer() : _stop_srv(false)
{
    /* Configuration de l'adresse du serveur */
    _srv_addr.sin_family = AF_INET;
    _srv_addr.sin_addr.s_addr = INADDR_ANY;
    _srv_addr.sin_port = htons(SOCKS_PORT);
}

void SocksServer::shutdown(int sig_num)
{
    /* Exemple de condition arbitraire de fermeture du serveur basée sur une durée*/
    std::this_thread::sleep_for(std::chrono::seconds(sig_num *60));
    _stop_srv = true;
}

int SocksServer::start()
{
    int _client_fd = 0;                              /* Récuère le fd client */
    sockaddr_in _client_addr = {0};                  /* Structure sockaddr_in de l'adresse du client */
    socklen_t _client_len = sizeof(_client_addr);    /* Taille */
    int exit = -1;                                   /* Valeur de retour de la fonction */
    int alive = 2;                                   /* Durée d'exécution du serveur (min) */

    /* Crée un socket */
    _srv_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( _srv_fd == -1)
    {
        std::cerr << "Erreur de création de socket" << std::endl;
    }
    else
    {
        /* Lie le socket au port */
        if (bind(_srv_fd, (sockaddr *)&_srv_addr, sizeof(_srv_addr)) == -1)
        {
            std::cerr << "Erreur de bind" << std::endl;
        }
        else
        {
            /* Écoute les connexions entrantes */
            if (listen(_srv_fd, SOMAXCONN) == -1)
            {
                std::cerr << "Erreur d'écoute" << std::endl;
            }
            else
            {
                std::cout << "Serveur SOCKS en écoute sur le port " << SOCKS_PORT << std::endl;
                exit = 1;
            }
        }
    }

    /* Ferme le server si la mise en place du serveur a échouée */
    if(exit != -1)
    {
        /* Lancement du thread permettant la fermuture du serveur */
        std::thread stop_thread(&SocksServer::shutdown, this, alive);

        /* Tant que la condition d'arrêt du client n'est pas remplie */
        while (not _stop_srv)
        {
            /* Accepte les connexions des clients */
            _client_fd = accept(_srv_fd, (sockaddr *)&_client_addr, &_client_len);

            /* Lance un thread de gestion de chaque client valide */
            if(_client_fd == -1)
            {
                perror("Echec acceptation de connection");
            }
            else if (_client_fd > 0)
            {
                std::thread(SocksServer::hdlClient, _client_fd).detach();
            }
        }
        stop_thread.join();
    }

    close(_srv_fd);

    return exit;
}

void SocksServer::hdlClient(int client_fd)
{
    SocksClient client(client_fd);

    /* Lancer la gestion du client */
    client.start();
}
