#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>


#pragma comment(lib, "ws2_32.lib")

namespace TCC{
    class UdpSender {
    public:
        UdpSender(const std::string& ip, int port);
        ~UdpSender();

        bool init();
        void sendByteData(const char * buffer, int length);

    private:
        std::string ip_;
        int port_;
        SOCKET sock_;
        sockaddr_in targetAddr_;
    };
}