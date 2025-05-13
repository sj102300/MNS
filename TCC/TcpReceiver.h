#pragma once

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

namespace TCC {
    class TcpReceiver {
    public:
        TcpReceiver(std::string ip_address, int port);

        // 초기화: 소켓 생성, 바인딩 등
        bool init();

        // 클라이언트 연결 수락
        bool connectClient();

        //recv 스레드 시작시 한번만 호출
        virtual void start() = 0;

        virtual void receive() = 0;

        // 응답 처리
        virtual void response() = 0;

        // 소켓 핸들링 관련 로직
        //virtual void getSocket();

    protected:
        SOCKET serverSocket_;
        SOCKET clientSocket_;
        sockaddr_in serverAddr_;
        sockaddr_in clientAddr_;
        int clientAddrLen_;

        std::string ip_address_;
        int port_;
    };
}