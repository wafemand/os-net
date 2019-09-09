#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <util/UDPSocket.h>
#include "TFTPServer.h"


#include <cstring>


using namespace std;
using namespace TFTP;


void print_usage() {
    cout << 
"./server_app port\n \
server opens on given port and give access to curent directory for network" << endl;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return EXIT_FAILURE;
    }

    const int port = stoi(argv[1]);

    TFTPServer server(port);
    server.run();
}
