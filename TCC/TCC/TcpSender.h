#pragma once

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

namespace TCC {
    class TcpSender {
    public:
        TcpSender(std::string ip_address, int port);

        // 초기화: 소켓 생성 등
        bool init();

        // 서버와 연결 요청
        bool connectToServer();

        // 데이터 송신
        bool sendByteData(const char* data, int length);

        // 연결 종료
        void disconnect();

    protected:
        SOCKET clientSocket_;
        SOCKET serverSocket_;
        sockaddr_in clientAddr_;
        sockaddr_in serverAddr_;
        int clientAddrLen_;

        std::string ip_address_;
        int port_;
    };
}