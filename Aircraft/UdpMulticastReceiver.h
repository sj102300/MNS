#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include "Missile.h"

#pragma comment(lib, "ws2_32.lib")

class UdpMulticastReceiver {
public:
    UdpMulticastReceiver();
    ~UdpMulticastReceiver();

    bool init(const std::string& multicast_address, int port);
    ParsedMissileData receiveMissile();  // �ѹ濡 �̻��� ���� ��ȯ

private:
    SOCKET sock_;
    sockaddr_in localAddr_;
    ip_mreq mreq_;
};  
