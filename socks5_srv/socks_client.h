#ifndef SOCKSCLIENT_H
#define SOCKSCLIENT_H

#include "socks_server.h"

/* Classe de gestion d'un client */
class SocksClient
{
private:
    /* Membres de la classe */
    sockaddr_in _target_addr;
    unsigned char _buff[INET6_ADDRSTRLEN];
    int _client_fd;
    int _target_fd;

    /* Etapes de communication */
    int checkAuth();
    void processReq();

    /* Méthode d'envoi d'une réponse d'erreur avec le code d'erreur correspondant */
    void sendRepErr(unsigned char rep_code);


    /* Implementation de la méthode CONNECT telle que défini dans le protocole SOCKS */
    void connectMtd(int client_fd);
    int relayData(int in_fd, int out_fd);

public:

    SocksClient(int client_fd);

    /* Méthode pour démarrer la gestion de la communication du client */
    void start();

};

#endif // SOCKSCLIENT_H
