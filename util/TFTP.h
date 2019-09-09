#pragma once

#include <utility>

#include <algorithm>
#include <iostream>
#include <sstream>
#include "UDPSocket.h"


namespace TFTP {
    inline uint16_t getUint16(Byte a, Byte b) {
        return b + ((uint16_t) a << 8u);
    }


    inline std::vector<Byte> uint16ToVec(uint16_t value) {
        return {
                Byte(value >> 8u),
                Byte(value & 0xffu)
        };
    }


    enum PacketType {
        RRQ = 1,
        WRQ = 2,
        DATA = 3,
        ACK = 4,
        ERROR = 5
    };


    inline std::string getName(PacketType type) {
        switch (type) {
            case RRQ:
                return "RRQ";
            case WRQ:
                return "WRQ";
            case DATA:
                return "DATA";
            case ACK:
                return "ACK";
            case ERROR:
                return "ERROR";
        }
        return "BULLSHIT";
    }


    inline bool isCorrectType(uint16_t typeNum) {
        return 1 <= typeNum && typeNum <= 5;
    }


    enum ErrorCode {
        Unknown = 0,
        FileNotFound = 1,
        AccessViolation = 2,
        DiskFull = 3,
        IllegalOperation = 4,
        UnknownTID = 5,
        FileExists = 6,
        NoSuchUser = 7
    };


    struct RRQPacket {
        std::string filename;
        std::string mode;
    };


    inline RRQPacket parseRRQ(std::vector<Byte> const &data) {
        RRQPacket res;
        auto zeroIt = std::find(data.begin() + 2, data.end(), '\0');
        res.filename = std::string(data.begin() + 2, zeroIt);
        res.mode = std::string(zeroIt + 1, data.end() - 1);
        return res;
    }


    inline std::vector<Byte> packRRQ(RRQPacket const &packet) {
        auto res = uint16ToVec(RRQ);
        res.insert(res.end(), packet.filename.begin(), packet.filename.end());
        res.push_back(0);
        res.insert(res.end(), packet.mode.begin(), packet.mode.end());
        return res;
    }


    struct WRQPacket {
        std::string filename;
        std::string mode;
    };


    inline WRQPacket parseWRQ(std::vector<Byte> const &data) {
        RRQPacket rrq = parseRRQ(data);
        return {rrq.filename, rrq.mode};
    }


    inline std::vector<Byte> packWRQ(WRQPacket const &packet) {
        auto res = uint16ToVec(WRQ);
        res.insert(res.end(), packet.filename.begin(), packet.filename.end());
        res.push_back(0);
        res.insert(res.end(), packet.mode.begin(), packet.mode.end());
        return res;
    }


    struct DataPacket {
        uint16_t blockNumber = 0;
        std::vector<Byte> data;
    };


    inline DataPacket parseDATA(std::vector<Byte> const &data) {
        DataPacket res;
        res.blockNumber = getUint16(data[2], data[3]);
        res.data = std::vector<Byte>(data.begin() + 4, data.end());
        return res;
    }


    inline std::vector<Byte> packDATA(DataPacket const &packet) {
        auto res = uint16ToVec(DATA);
        auto blockNum = uint16ToVec(packet.blockNumber);
        res.insert(res.end(), blockNum.begin(), blockNum.end());
        res.insert(res.end(), packet.data.begin(), packet.data.end());
        return res;
    }


    struct ACKPacket {
        uint16_t blockNumber = 0;
    };


    inline ACKPacket parseACK(std::vector<Byte> const &data) {
        ACKPacket res;
        res.blockNumber = getUint16(data[2], data[3]);
        return res;
    }


    inline std::vector<Byte> packACK(ACKPacket const &packet) {
        auto res = uint16ToVec(ACK);
        auto blockNum = uint16ToVec(packet.blockNumber);
        res.insert(res.end(), blockNum.begin(), blockNum.end());
        return res;
    }


    struct ErrorPacket {
        uint16_t errorCode = 0;
        std::string errorMessage;
    };


    inline ErrorPacket parseERROR(std::vector<Byte> const &data) {
        ErrorPacket res;
        res.errorCode = getUint16(data[2], data[3]);
        res.errorMessage = std::string(data.begin() + 4, data.end() - 1);
        return res;
    }


    inline std::vector<Byte> packERROR(ErrorPacket const &packet) {
        auto res = uint16ToVec(ERROR);
        auto errorCode = uint16ToVec(packet.errorCode);
        res.insert(res.end(), errorCode.begin(), errorCode.end());
        res.insert(res.end(), packet.errorMessage.begin(), packet.errorMessage.end());
        return res;
    }


    class TFTPError : public std::exception {
        std::string msg;
    public:
        explicit TFTPError(ErrorPacket const &packet) {
            std::stringstream temp;
            temp << "TFTP Error #" << packet.errorCode << ":\n";
            temp << packet.errorMessage;
            msg = temp.str();
        }

        char const *what() const noexcept override {
            return msg.c_str();
        }
    };


    class TFTPSocket {
    public:
        static const int DATA_MAX_SIZE = 512;
        static const int MAX_RESENDS = 10;


        TFTPSocket() = default;

        explicit TFTPSocket(in_port_t port) : udpSocket(port) {}


        void sendRRQ(UDPAddress address, std::string filename, std::string mode) {
//            std::cerr << "send RRQ: " << filename << std::endl;
            RRQPacket rrqPacket{std::move(filename), std::move(mode)};
            send(address, packRRQ(rrqPacket));
        }


        void sendWRQ(UDPAddress address, std::string filename, std::string mode) {
//            std::cerr << "send WRQ: " << filename << std::endl;
            WRQPacket wrqPacket{std::move(filename), std::move(mode)};
            send(address, packWRQ(wrqPacket));
        }


        void sendACK(UDPAddress address, uint16_t blockNumber) {
//            std::cerr << "send ACK: " << blockNumber << std::endl;
            ACKPacket ackPacket{blockNumber};
            send(address, packACK(ackPacket));
        }


        void sendERROR(UDPAddress address, uint16_t errorCode, std::string message) {
//            std::cerr << "send ERROR: " << errorCode << " " << message << std::endl;
            ErrorPacket errorPacket{errorCode, std::move(message)};
            send(address, packERROR(errorPacket));
        }


        void waitACKFrom(uint16_t blockNumber, UDPAddress address) {
//            std::cerr << "Wait ACK: " << blockNumber << std::endl;
            waitFrom(blockNumber, address, ACK);
        }


        std::vector<Byte> waitDATAFrom(uint16_t blockNumber, UDPAddress address) {
//            std::cerr << "Wait DATA: " << blockNumber << std::endl;
            return parseDATA(waitFrom(blockNumber, address, DATA).data).data;
        }


        bool sendFile(UDPAddress address, FILE *file) {
            int bytes;
            Byte buffer[DATA_MAX_SIZE];

            int blockNumber = 1;
            while (true) {
                bytes = fread(buffer, 1, DATA_MAX_SIZE, file);
                if (bytes != DATA_MAX_SIZE && ferror(file)) {
                    sendERROR(address, Unknown, strerror(errno));
                    return false;
                }
                auto data = std::vector<Byte>(buffer, buffer + bytes);

                sendDATA(address, blockNumber, data);

                for (int tryNum = 0; tryNum < MAX_RESENDS; tryNum++) {
                    try {
                        waitACKFrom(blockNumber, address);
                        blockNumber++;
                        break;
                    } catch (TimeOutException const &e) {
                        sendDATA(address, blockNumber, data);
                        continue;
                    }
                }

                if (feof(file)) {
                    return true;
                }
            }
        }


        bool receiveFile(UDPAddress address, FILE *file, bool resendZeroACK) {
            int blockNumber = 0;
            while (true) {
                std::vector<Byte> data;

                for (int tryNum = 0; tryNum < MAX_RESENDS; tryNum++) {
                    try {
                        data = waitDATAFrom(blockNumber + 1, address);
                        blockNumber++;
                        sendACK(address, blockNumber);
                        break;
                    } catch (TimeOutException const &e) {
                        if (!resendZeroACK && blockNumber == 0) {
                            throw e;
                        } else {
                            sendACK(address, blockNumber);
                        }
                        continue;
                    }
                }

                int bytes = fwrite(data.data(), 1, data.size(), file);
                if (bytes != data.size()) {
                    sendERROR(address, Unknown, strerror(errno));
                    return false;
                }

                if (data.size() != DATA_MAX_SIZE) {
                    return true;
                }
            }
        }


        PacketType receivePacket(ReceiveData &request) {
            while (true) {
                request = udpSocket.receive();
                auto typeNum = getUint16(request.data[0], request.data[1]);
                if (!isCorrectType(typeNum)) {
                    sendERROR(request.senderAddress, IllegalOperation, "Unknown packet type.");
                    continue;
                }
//                std::cerr << "receive packet: " << getName(PacketType(typeNum)) << std::endl;
                return PacketType(typeNum);
            }
        }

    private:
        ReceiveData waitFrom(uint16_t blockNumber, UDPAddress address, PacketType expected) {
            while (true) {
                ReceiveData receiveData;
                PacketType type = receivePacket(receiveData);
                if (receiveData.senderAddress != address) {
                    sendERROR(receiveData.senderAddress, UnknownTID,
                              "Server processes another client.");
                    continue;
                }
                if (type == ERROR) {
                    throw TFTPError(parseERROR(receiveData.data));
                }
                if (type == expected) {
                    // I hope ACK and DATA packet always will have similar format :)
                    ACKPacket ackPacket = parseACK(receiveData.data);
                    if (ackPacket.blockNumber == blockNumber) {
                        return receiveData;
                    }
                }
            }
        }


        void sendDATA(UDPAddress address, uint16_t blockNumber, std::vector<Byte> data) {
//            std::cerr << "send DATA: " << blockNumber << std::endl;
            DataPacket dataPacket{blockNumber, std::move(data)};
            send(address, packDATA(dataPacket));
        }


        void send(UDPAddress address, const std::vector<Byte> &data) {
            try {
                udpSocket.send(data, address);
            } catch (UDPException const &e) {
                std::cerr << e.what();
            }
        }

        UDPSocket udpSocket;
    };
}
