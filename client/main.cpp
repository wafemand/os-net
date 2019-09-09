#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <util/UDPSocket.h>
#include "TFTPClient.h"
#include <arpa/inet.h>


#include <cstring>
#include <netdb.h>


void printUsage();
using namespace std;
using namespace TFTP;


in_addr_t hostname_to_ip(string const &hostname) {
    auto he = gethostbyname(hostname.c_str());

    if (he == nullptr) {
        return 0;
    }

    auto **addr_list = (in_addr **) he->h_addr_list;

    return addr_list[0]->s_addr;
}


void printUsage() {
    cout << 
"./client_app hostname port action source_file destination_file\n \
    action:\n \
        download - download from remote path 'source_file' to local path 'destination_file'\n \
        upload - upload from local path 'source_file' to remote path 'destination_file'" << endl;
}


int main(int argc, char *argv[]) {
    if (argc < 6) {
        printUsage();
        return EXIT_FAILURE;
    }
    const in_addr_t address = hostname_to_ip(argv[1]);
    const in_port_t port = stoi(argv[2]);
    const string action = argv[3];
    
    if (address == 0) {
        cerr << "Cannot find host." << std::endl;
        return EXIT_FAILURE;
    }

    TFTPClient client;

    if (action == "download") {
        FILE *file = fopen(argv[5], "w");
        client.download(UDPAddress(address, port), argv[4], file);
        fclose(file);
    } else if (action == "upload") {
        FILE *file = fopen(argv[4], "r");
        client.upload(UDPAddress(address, port), file, argv[5]);
        fclose(file);
    } else {
        printUsage();
        return EXIT_FAILURE;
    }
    return 0;
}
