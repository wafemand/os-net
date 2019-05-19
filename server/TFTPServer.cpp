#include "TFTPServer.h"
#include <unistd.h>
#include <util/TFTP.h>
#include <fcntl.h>

namespace TFTP {
    TFTPServer::TFTPServer(in_port_t port)
            : tftpSocket(port),
              rootDir(get_current_dir_name()) {}


    void TFTPServer::run() {
        while (true) {
            ReceiveData request;
            auto type = tftpSocket.receivePacket(request);
            if (type == RRQ) {
                processRRQ(request);
            } else if (type == WRQ) {
                processWRQ(request);
            } else {
                tftpSocket.sendERROR(request.senderAddress, IllegalOperation, "Expect RRQ/WRQ request");
            }
        }
    }


    void TFTPServer::processRRQ(ReceiveData const &request) {
        auto packet = parseRRQ(request.data);

        FILE *file = fopen(packet.filename.c_str(), "r");

        if (file == nullptr) {
            if (errno == EACCES) {
                tftpSocket.sendERROR(request.senderAddress, AccessViolation, strerror(errno));
            } else if (errno == ENOENT) {
                tftpSocket.sendERROR(request.senderAddress, FileNotFound, strerror(errno));
            } else {
                tftpSocket.sendERROR(request.senderAddress, Unknown, strerror(errno));
            }
            return;
        }

        try {
            if (!tftpSocket.sendFile(request.senderAddress, file)) {
                std::cerr << "Cannot send file." << std::endl;
            }
        } catch (TFTPError const &e) {
            std::cerr << "Client send error. LoL" << std::endl;
        }
        fclose(file);
    }


    void TFTPServer::processWRQ(ReceiveData const &request) {
        auto packet = parseWRQ(request.data);

        FILE *file = fopen(packet.filename.c_str(), "w");

        if (file == nullptr) {
            if (errno == EACCES) {
                tftpSocket.sendERROR(request.senderAddress, AccessViolation, strerror(errno));
            } else {
                tftpSocket.sendERROR(request.senderAddress, Unknown, strerror(errno));
            }
            return;
        }

        tftpSocket.sendACK(request.senderAddress, 0);

        try {
            tftpSocket.receiveFile(request.senderAddress, file, true);
        } catch (TFTPError const &e) {
            std::cerr << "Client send error. LoL" << std::endl;
        }
        fclose(file);
    }
}


