#pragma once
#include "UdpReceiver.h"

UdpReceiver::UdpReceiver()
    : sock_(INVALID_SOCKET), running_(false), port_(0) {
}

UdpReceiver::~UdpReceiver() {
    close();
}

bool UdpReceiver::init(const std::string& multicast_addr, int port) {
    port_ = port;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "socket() failed\n";
        WSACleanup();
        return false;
    }

    int reuse = 1;
    if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt SO_REUSEADDR failed\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    memset(&localAddr_, 0, sizeof(localAddr_));
    localAddr_.sin_family = AF_INET;
    localAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr_.sin_port = htons(port_);

    if (bind(sock_, (sockaddr*)&localAddr_, sizeof(localAddr_)) < 0) {
        std::cerr << "bind() failed\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    mreq_.imr_multiaddr.s_addr = inet_addr(multicast_addr.c_str());
    mreq_.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_, sizeof(mreq_)) < 0) {
        std::cerr << "setsockopt IP_ADD_MEMBERSHIP failed\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    return true;
}

int UdpReceiver::receive(char* buffer, int bufferSize) {
    sockaddr_in senderAddr;
    int senderLen = sizeof(senderAddr);
    int ret = recvfrom(sock_, buffer, bufferSize, 0, (sockaddr*)&senderAddr, &senderLen);
    if (ret == SOCKET_ERROR) {
        std::cerr << "recvfrom failed: " << WSAGetLastError() << "\n";
        return -1;
    }
    return ret;
}

void UdpReceiver::close() {
    if (sock_ != INVALID_SOCKET) {
        setsockopt(sock_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq_, sizeof(mreq_));
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
    WSACleanup();
}
