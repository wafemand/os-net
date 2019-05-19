#pragma once


#include <string>
#include <util/UDPSocket.h>
#include <util/TFTP.h>

namespace TFTP {

    class TFTPClient {
    public:
        TFTPClient() = default;

        bool download(UDPAddress serverAddress, std::string const &remoteFilename, FILE *localFile);

        bool upload(UDPAddress serverAddress, FILE *localFile, const std::string& remoteFilename);

    private:
        TFTPSocket tftpSocket;
    };

}