#include <iostream>
#include "socks_server.h"


int main() {

    SocksServer my_server;

    int err = 0;
    err = my_server.start();

    return err;
}
