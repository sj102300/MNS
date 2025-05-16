#pragma once

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

namespace TCC {
    class TcpSender {
    public:
        TcpSender(std::string ip_address, int port);

        // 초기화: 소켓 생성 등
        bool init();

        // 서버와 연결 요청
        bool connectToServer();

        // 데이터 송신
        virtual bool sendData(const char* data, int length) = 0;

        // 요청 수신
        virtual void getResponse() = 0;

        // 쓰레드 생성. 클라이언트 생성
        virtual void start() = 0;

        // 연결 종료
        virtual void disconnect() = 0;

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