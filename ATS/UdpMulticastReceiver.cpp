#include "UdpMulticastReceiver.h"
#include <iostream>
#include <cstring>  // for memset

UdpMulticastReceiver::UdpMulticastReceiver() : sock_(INVALID_SOCKET) {}

UdpMulticastReceiver::~UdpMulticastReceiver() {
    if (sock_ != INVALID_SOCKET) {
        setsockopt(sock_, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq_, sizeof(mreq_));
        closesocket(sock_);
        WSACleanup();
    }
}

bool UdpMulticastReceiver::init(const std::string& multicast_address, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        return false;
    }

    int reuse = 1;
    setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    memset(&localAddr_, 0, sizeof(localAddr_));
    localAddr_.sin_family = AF_INET;
    localAddr_.sin_port = htons(port);
    localAddr_.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_, (sockaddr*)&localAddr_, sizeof(localAddr_)) < 0) {
        std::cerr << "Bind failed\n";
        return false;
    }

    inet_pton(AF_INET, multicast_address.c_str(), &mreq_.imr_multiaddr);
    mreq_.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_, sizeof(mreq_)) < 0) {
        std::cerr << "setsockopt(IP_ADD_MEMBERSHIP) failed\n";
        return false;
    }

    u_long mode = 1;
    ioctlsocket(sock_, FIONBIO, &mode);

    //std::cout << "Multicast receiver initialized on " << multicast_address << ":" << port << "\n";
    return true;
}

std::unordered_map<std::string, ParsedMissileData> UdpMulticastReceiver::receiveAllMissiles() {
    std::unordered_map<std::string, ParsedMissileData> missiles;
    Missile rawPacket{};
    sockaddr_in senderAddr{};
    int senderLen = sizeof(senderAddr);

    while (true) {
        int recvLen = recvfrom(sock_, reinterpret_cast<char*>(&rawPacket), sizeof(rawPacket), 0,
            (sockaddr*)&senderAddr, &senderLen);
        if (recvLen <= 0) break;
        if (rawPacket.eventCode != 3001) continue;

        std::string idStr(rawPacket.missileId, 8);
        auto nullPos = idStr.find('\0');
        if (nullPos != std::string::npos) idStr.erase(nullPos);

        ParsedMissileData data;
        data.eventCode = rawPacket.eventCode;
        data.missileId = idStr;
        data.latitude = rawPacket.latitude;
        data.longitude = rawPacket.longitude;
        data.altitude = rawPacket.altitude;

        missiles[idStr] = data;
    }

    return missiles;
}
