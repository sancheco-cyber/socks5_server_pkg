#ifndef SOCKSSERVER_H
#define SOCKSSERVER_H

#include <netinet/in.h>


/* Informations relatives au protocole SOCKS5 tel que défini dans la RFC 1928 */
#define SOCKS_PORT 1080

#define SOCKS_VERSION (0x05)
#define SOCKS_NO_AUTH_METHOD  (0x00)

#define SOCKS_ATYP_IPV4     (0x01)
#define SOCKS_ATYP_DOMNAME  (0x03)
#define SOCKS_ATYP_IPV6     (0x04)

#define SOCKS_CMD_CONNECT  (0x01)
#define SOCKS_CMD_BIND     (0x02)
#define SOCKS_CMD_UDPASS   (0x03)

#define SOCKS_RSV   (0x00)

#define SOCKS_REP_SUCCESS       (0x00)
#define SOCKS_REP_SOCKS_ERROR   (0x01)
#define SOCKS_REP_PERM_DENIED   (0x02)
#define SOCKS_REP_NET_UNR       (0x03)
#define SOCKS_REP_HOST_UNR      (0x04)
#define SOCKS_REP_CONN_REF      (0x05)
#define SOCKS_REP_TTL_TIMEOUT   (0x06)
#define SOCKS_REP_CMD_ERR       (0x07)
#define SOCKS_REP_ATYP_ERR      (0x08)


/* Classe du serveur SOCKS */
class SocksServer
{
private:
    /* Information du serveur */
    int _srv_fd;
    sockaddr_in _srv_addr;

    /* Durée de vie du serveur (min) */
    int _life;

    /* Membre permettant d'arrêter le serveur */
    bool _stop_srv;

    /* Methode arbitraire de fermeture su serveur */
    void shutdown(int sig_num);

    /* Methode permettant de lancer la gestion de chaque client dans un thread */
    static void hdlClient(int client_fd);

public:
    SocksServer(int life);

    /* Fonction de démarrage du client */
    int start();
};

#endif // SOCKSSERVER_H
