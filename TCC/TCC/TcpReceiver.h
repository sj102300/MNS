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

        // �ʱ�ȭ: ���� ����, ���ε� ��
        bool init();

        // Ŭ���̾�Ʈ ���� ����
        bool connectClient();

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