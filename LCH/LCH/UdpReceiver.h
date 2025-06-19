#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <iostream>

class UdpReceiver {
public:
    UdpReceiver();
    ~UdpReceiver();

    bool init(const std::string& multicast_addr, int port);
    void close();

    // blocking recvfrom 함수 (스레드 없이도 호출 가능)
    int receive(char* buffer, int bufferSize);

private:
    SOCKET sock_;
    sockaddr_in localAddr_;
    ip_mreq mreq_;
    bool running_;
    int port_;
};