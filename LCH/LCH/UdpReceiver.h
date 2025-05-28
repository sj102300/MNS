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

    // blocking recvfrom �Լ� (������ ���̵� ȣ�� ����)
    int receive(char* buffer, int bufferSize);

private:
    SOCKET sock_;
    sockaddr_in localAddr_;
    ip_mreq mreq_;
    bool running_;
    int port_;
};