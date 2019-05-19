#pragma once

#include <string>
#include <netinet/in.h>
#include <map>
#include <util/UDPSocket.h>
#include <util/TFTP.h>

namespace TFTP {
    class TFTPServer {
    public:

        explicit TFTPServer(in_port_t port);

        void run();

    private:

        void processRRQ(ReceiveData const &request);

        void processWRQ(ReceiveData const &request);

        TFTPSocket tftpSocket;
        std::string rootDir;
    };
}