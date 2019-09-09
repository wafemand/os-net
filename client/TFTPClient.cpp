//
// Created by andrey on 18/05/19.
//

#include "TFTPClient.h"


namespace TFTP {
    bool TFTPClient::download(UDPAddress serverAddress,
                              const std::string &remoteFilename,
                              FILE *localFile) {
        tftpSocket.sendRRQ(serverAddress, remoteFilename, "octet");

        for (int tryNum = 0; tryNum < TFTPSocket::MAX_RESENDS; tryNum++) {
            try {
                return tftpSocket.receiveFile(serverAddress, localFile, false);
            } catch (TimeOutException const &e) {
                tftpSocket.sendRRQ(serverAddress, remoteFilename, "octet");
            } catch (TFTPError const &e) {
                std::cerr << e.what();
                return false;
            }
        }
        return false;
    }

    bool TFTPClient::upload(UDPAddress serverAddress,
                            FILE *localFile,
                            const std::string &remoteFilename) {
        tftpSocket.sendWRQ(serverAddress, remoteFilename, "octet");

        try {
            tftpSocket.waitACKFrom(0, serverAddress);
        } catch (TimeOutException const &e) {
            std::cerr << "Server doesn't responding" << std::endl;
            return false;
        } catch (TFTPError const &e) {
            std::cerr << e.what();
            return false;
        }

        return tftpSocket.sendFile(serverAddress, localFile);
    }
}
