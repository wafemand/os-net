#pragma once

#include <vector>
#include <string>
#include <netinet/in.h>
#include <cstring>


typedef uint8_t Byte;


class UDPException : public std::exception {
    std::string msg;
public:
    explicit UDPException(std::string const &msg, error_t error)
            : msg(msg + "\nError:" + strerror(error)) {}

    char const *what() const noexcept override {
        return msg.c_str();
    }
};


class TimeOutException : public  std::exception {};


struct UDPAddress {
    in_addr_t address;
    in_port_t port;

    UDPAddress() = default;
    UDPAddress(in_addr_t address, in_port_t port);

    in_addr_t networkByteOrderAddress();
    in_port_t networkByteOrderPort();
};


bool operator< (UDPAddress a, UDPAddress b);

bool operator!= (UDPAddress a, UDPAddress b);

bool operator== (UDPAddress a, UDPAddress b);


struct ReceiveData {
    std::vector<Byte> data;
    UDPAddress senderAddress;
};


class UDPSocket {
public:
    explicit UDPSocket(UDPAddress socketAddress);

    explicit UDPSocket(in_port_t port);

    UDPSocket();

    ~UDPSocket();

    void bind(UDPAddress socketAddress);

    ReceiveData receive();

    void send(std::vector<Byte> const &message, UDPAddress socketAddress);

private:
    int socketFD;

    static const int BUFSIZE = 1024 * 128;
};