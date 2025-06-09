#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <string>
#include <iostream>

class UdpMulSender {
public:
    UdpMulSender();
    ~UdpMulSender();

    bool init(const std::string& multicast_addr, int port);
    int send(const char* data, int length);
    void close();

private:
    SOCKET sock_;
    sockaddr_in destAddr_;
};
