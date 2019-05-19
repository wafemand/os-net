#include <cerrno>
#include "UDPSocket.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>


using namespace std;


UDPAddress::UDPAddress(in_addr_t address, in_port_t port)
        : address(address), port(port) {}

in_addr_t UDPAddress::networkByteOrderAddress() {
    return htons(address);
}

in_port_t UDPAddress::networkByteOrderPort() {
    return htons(port);
}


UDPSocket::UDPSocket(UDPAddress socketAddress) : UDPSocket() {
    bind(socketAddress);
}


UDPSocket::UDPSocket(in_port_t port) : UDPSocket(UDPAddress(INADDR_ANY, port)) {}


UDPSocket::UDPSocket() {
    socketFD = socket(AF_INET, SOCK_DGRAM, 0);

    if (socketFD == -1) {
        throw UDPException("Cannot create socket.", errno);
    }
}


UDPSocket::~UDPSocket() {
    close(socketFD);
}


void UDPSocket::bind(UDPAddress socketAddress) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = socketAddress.networkByteOrderPort();
    addr.sin_addr.s_addr = socketAddress.networkByteOrderAddress();

    if (::bind(socketFD, (sockaddr *) &addr, sizeof(sockaddr_in)) == -1) {
        throw UDPException("Cannot bind to address.", errno);
    }

//    std::cerr << "Server started at: " << socketAddress.address << ' ' << socketAddress.port << std::endl;
}

ReceiveData UDPSocket::receive() {
    std::vector<Byte> res(BUFSIZE);
    sockaddr_in src{};
    socklen_t len = sizeof(src);
    int bytes = recvfrom(socketFD, res.data(), BUFSIZE, 0, (sockaddr *) &src, &len);

    if (bytes == -1) {
        if (errno == EAGAIN) {
            throw TimeOutException();
        } else {
            throw UDPException("Cannot receive packet.", errno);
        }
    }

    UDPAddress address(src.sin_addr.s_addr, htons(src.sin_port));
//    cerr << "Src: " << address.address << ' ' << address.port << endl;
    res.resize(bytes);

    return {res, address};
}


void UDPSocket::send(std::vector<Byte> const &message, UDPAddress socketAddress) {
    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = socketAddress.networkByteOrderPort();
    //dest.sin_port = socketAddress.port;
    //dest.sin_addr.s_addr = socketAddress.networkByteOrderAddress();
    dest.sin_addr.s_addr = socketAddress.address;
    int len = sizeof(sockaddr_in);

//    cerr << "Dest: " << socketAddress.address << ' ' << socketAddress.port << endl;
    int bytes = sendto(socketFD, message.data(), message.size(), 0, (sockaddr *) &dest, len);

    if (bytes == -1) {
        throw UDPException("Cannot send packet.", errno);
    }
}

bool operator<(UDPAddress a, UDPAddress b) {
    return a.address == b.address ? a.address < b.address : a.port < b.port;
}

bool operator!=(UDPAddress a, UDPAddress b) {
    return a.port != b.port || a.address != b.address;
}

bool operator==(UDPAddress a, UDPAddress b) {
    return a.port == b.port && a.address == b.address;
}
