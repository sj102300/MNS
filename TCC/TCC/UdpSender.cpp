#include "UdpSender.h"

TCC::UdpSender::UdpSender(const std::string& ip, int port)
    : ip_(ip), port_(port), sock_(INVALID_SOCKET) {
    ZeroMemory(&targetAddr_, sizeof(targetAddr_));
}

TCC::UdpSender::~UdpSender() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
    }
    WSACleanup();
}

bool TCC::UdpSender::init() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return false;
    }

    targetAddr_.sin_family = AF_INET;
    targetAddr_.sin_port = htons(port_);
    targetAddr_.sin_addr.s_addr = inet_addr(ip_.c_str());

    return true;
}

void TCC::UdpSender::sendByteData(const char* data, int length) {
    int bytesSent = sendto(sock_, data, length, 0,
        (sockaddr*)&targetAddr_, sizeof(targetAddr_));
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "sendto() failed: " << WSAGetLastError() << "\n";
    }
}

//
//bool UdpSender::sendMessage(const std::string& message) {
//    if (!initialized_) {
//        std::cerr << "UdpSender not initialized\n";
//        return false;
//    }
//
//    int result = sendto(sock_, message.c_str(), message.size(), 0,
//        (sockaddr*)&targetAddr_, sizeof(targetAddr_));
//    if (result == SOCKET_ERROR) {
//        std::cerr << "sendto() failed: " << WSAGetLastError() << "\n";
//        return false;
//    }
//
//    return true;
//}
