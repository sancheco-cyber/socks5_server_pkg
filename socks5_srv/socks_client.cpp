#include "socks_client.h"

#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>

#include <iostream>

SocksClient::SocksClient(int client_fd) : _client_fd(client_fd)
{

}

void SocksClient::start()
{
    std::cout << "AUTHENTIFICATION" << std::endl;

    if(checkAuth())
    {
        std::cout << "PROCESS REQ" << std::endl;
        processReq();
    }

    close(_client_fd);
    close(_target_fd);

    std::cout << "CLOSED CLIENT" << std::endl;
}

int SocksClient::checkAuth()
{
    bool success = false;   /* True si l'authentification a reussie */
    ssize_t data_len = -1;  /* Taille de la réponse envoyée */
    unsigned char resp_no_auth[2] = {SOCKS_VERSION, SOCKS_NO_AUTH_METHOD}; /* Message de réponse d'authentification */

    /* Reception de la première trame */
    data_len = recv(_client_fd, _buff, sizeof (_buff), 0);

    /* Traitement si données reçues */
    if(data_len > 0)
    {
        if ( _buff[0] == SOCKS_VERSION)
        {
            /* Si bonne version SOCKS envoi de la réponse (aucune authentification nécessaire) */
            data_len = send(_client_fd, resp_no_auth, sizeof (resp_no_auth), 0);
            if(data_len > 0)
            {
                /* Message envoyé */
                success = true;
            }
        }
        else
        {
            sendRepErr(SOCKS_REP_SOCKS_ERROR);
        }
    }
    return success;
}

void SocksClient::processReq()
{
    ssize_t data_len = -1; /* Taille de la requête recue */

    /* Recpetion de la requête */
    data_len = recv(_client_fd, _buff, sizeof (_buff), 0);
    if(data_len > 0)
    {
        /* Application de la commande demandée */
        switch (_buff[1])
        {
        case SOCKS_CMD_CONNECT:
            connectMtd(_client_fd);
            break;
        case SOCKS_CMD_BIND:
        case SOCKS_CMD_UDPASS:
        default :
        {
            std::cerr << "SOCKS CMD not implemented" << std::endl;
            sendRepErr(SOCKS_REP_CMD_ERR);
            break;
        }
        }
    }
    return;
}

void SocksClient::connectMtd(int client_fd)
{
    unsigned char dest_addr_len = 0;         /* Longueur de l'adresse de destination à résoudre */
    int start_dest_addr = 4;                 /* Index de début de l'adresse de destination */
    std::string dest_addr = "";              /* Adresse de destination sous forme de string xxx.xxx.xxx.xxx */
    char target_ip_adress[INET_ADDRSTRLEN];  /* Adresse IP de destination */
    unsigned short target_port = 0;          /* Port de destination */
    ssize_t data_len = -1;                   /* Taille des messages recus et envoyés */
    bool exit = true;                        /* Condition de sortie si l'authentification a échouée */
    unsigned char resp[10] = {SOCKS_VERSION, SOCKS_REP_SUCCESS, SOCKS_RSV, SOCKS_ATYP_IPV4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* Message de réponse en succès */

    /* Paramètres permettant la résolution du nom de domaine avec getaddrinfo */
    sockaddr_in * target_addr = nullptr;
    addrinfo hints = {0};
    addrinfo *res = nullptr;

    /* Analyse du type d'adresse */
    switch (_buff[3])
    {
    case SOCKS_ATYP_IPV4:
        dest_addr_len = 4;
        break;
    case SOCKS_ATYP_DOMNAME:
        /* Dans ce cas, le premier octet de l'adresse indique la longueur de celle-ci qui suit*/
        dest_addr_len = _buff[start_dest_addr];
        start_dest_addr ++;
        break;
    case SOCKS_ATYP_IPV6:
        dest_addr_len = 16;
        break;
    default:
        {
            sendRepErr(SOCKS_REP_ATYP_ERR);
            break;
        }
    }

    /* Connection à l'adresse cible si type d'adresse décodée */
    if(dest_addr_len != 0)
    {
        /* Connection a l'adrese cible */
        dest_addr = std::string(reinterpret_cast<char*>(&_buff[start_dest_addr]), dest_addr_len);
        target_port = (_buff[5+dest_addr_len] << 8) | _buff[6 + dest_addr_len];

        /* Résolution du nom de domaine avec getaddrinfo */
        memset(&hints, 0, sizeof (hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        data_len = getaddrinfo(dest_addr.c_str(), std::to_string(target_port).c_str(), &hints, &res);
        if (data_len != 0)
        {
            std::cerr << "Échec de la résolution du nom de domaine : " << gai_strerror(data_len) << std::endl;
            sendRepErr(SOCKS_REP_NET_UNR);
        }
        else
        {

            /* Affiche l'adresspe IP */
            target_addr = (sockaddr_in*)res->ai_addr;
            inet_ntop(AF_INET, &target_addr->sin_addr, target_ip_adress, sizeof (target_ip_adress));
            std::cout << "ip =" << target_ip_adress << " - port =" << target_port << " - adresse=" << dest_addr << std::endl;

            /* Crée un socket */
            _target_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (_target_fd == -1)
            {
                perror("Erreur de création de socket");
                sendRepErr(SOCKS_REP_HOST_UNR);
            }
            else
            {
                /* Lie le socket au port */
                if (connect(_target_fd, res->ai_addr, res->ai_addrlen) == -1) {
                    perror("Connection failed");
                    sendRepErr(SOCKS_REP_CONN_REF);
                }
                else
                {
                    /* Construction de la réponse avec les informations d'adresse et de port au client */
                    resp[4] = (unsigned char)(target_addr->sin_addr.s_addr) & 0xFF;
                    resp[5] = (unsigned char)(target_addr->sin_addr.s_addr >> 8) & 0xFF;
                    resp[6] = (unsigned char)(target_addr->sin_addr.s_addr >> 16) & 0xFF;
                    resp[7] = (unsigned char)(target_addr->sin_addr.s_addr >> 24) & 0xFF;
                    resp[8] = (unsigned char)(target_port >> 8) & 0xFF;
                    resp[9] = (unsigned char)(target_port) & 0xFF;

                    /* Envoi de la réponse */
                    data_len = send(_client_fd, resp, sizeof (resp), 0);

                    if(data_len > 0)
                    {
                        /* Message envoyé */
                        exit = false;
                    }
                }
            }
        }
    }

    std::cout << "DATA RELAY" << std::endl;

    /* Une fois la connection établie, en fait le relais entre le client et la cible */
    while(not exit)
    {
        exit = relayData(_client_fd, _target_fd);
        if(not exit)
        {
            exit = relayData(_target_fd, _client_fd);
        }
    }
    return;
}

int SocksClient::relayData(int in_fd, int out_fd)
{
    ssize_t data_len = -1;     /* Taille des messages recus et envoyés */
    bool exit = false;         /* Condition de sortie du relay des données */
    const int timeout = 100;   /* Temps d'attente */


    /* Le socket in est rendu non bloquant */
    pollfd p_in_fd = {0};
    p_in_fd.fd = in_fd;
    p_in_fd.events = POLLIN;

    /* Vérification de la validité des fd d'entrée */
    if(in_fd <= 0 or out_fd <= 0)
    {
        exit = true;
    }

    while(not exit)
    {
        /* Analyse de messages en attente sur in_fd */
        data_len = poll(&p_in_fd, 1, timeout);
        if(data_len <= 0)
        {
            if(data_len < 0)
            {
                perror("poll from target");
            }
            break;
        }
        else
        {
            /* Des données sont prêtes à être lues */
            if(p_in_fd.revents & POLLIN)
            {
                /* Lecture */
                data_len = recv(in_fd, _buff, sizeof (_buff), 0);
                if(data_len <= 0)
                {
                    /* Si err == 0 -> in_fd a fermé la connection */
                    if(data_len < 0)
                    {
                        perror("recv err");
                    }
                    exit = true;
                    break;
                }
                /* Relay des données recues vers out_fd */
                data_len = send(out_fd, _buff, data_len, 0);
                if(data_len <= 0)
                {
                    perror("send to out_fd");
                    exit = true;
                    break;
                }
            }
        }
    }
    return exit;
}

void SocksClient::sendRepErr(unsigned char rep_code)
{
    unsigned char resp[10] = {SOCKS_VERSION, 0x00, SOCKS_RSV, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  /* Message d'erreur */
    ssize_t data_len = -1;  /* Taille du message envoyé */

    resp[1] = rep_code;

    data_len = send(_client_fd, resp, sizeof (resp), 0);
}
