#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <util/UDPSocket.h>
#include "TFTPServer.h"


#include <cstring>


using namespace std;
using namespace TFTP;


int main(int argc, char *argv[]) {
    const int port = stoi(argv[1]);

    TFTPServer server(port);
    server.run();
}