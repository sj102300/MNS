#pragma once

#include <ws2tcpip.h>
#include "UdpMulSender.h"
#pragma comment(lib, "Ws2_32.lib")

UdpMulSender::UdpMulSender()
    : sock_(INVALID_SOCKET) {
}

UdpMulSender::~UdpMulSender() {
    close();
}

bool UdpMulSender::init(const std::string& multicast_addr, int port) {
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

    // TTL 설정 (멀티캐스트 패킷 전송 범위)
    char ttl = 1;
    if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        std::cerr << "setsockopt IP_MULTICAST_TTL failed\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    memset(&destAddr_, 0, sizeof(destAddr_));
    destAddr_.sin_family = AF_INET;
    if (inet_pton(AF_INET, "239.0.0.1", &destAddr_.sin_addr) <= 0) { // destAddr을 이제 멀티캐스트 주소로
        std::cerr << "Invalid multicast address format\n";
        closesocket(sock_);
        WSACleanup();
        return false;
    }
    destAddr_.sin_port = htons(port);

    return true;
}

int UdpMulSender::send(const char* data, int length) {
    int ret = sendto(sock_, data, length, 0, (sockaddr*)&destAddr_, sizeof(destAddr_));
    if (ret == SOCKET_ERROR) {
        std::cerr << "sendto failed: " << WSAGetLastError() << "\n";
        return -1;
    }
    return ret;
}

void UdpMulSender::close() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
    //WSACleanup();
}
