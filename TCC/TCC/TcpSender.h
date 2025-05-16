#pragma once

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

namespace TCC {
    class TcpSender {
    public:
        TcpSender(std::string ip_address, int port);

        // �ʱ�ȭ: ���� ���� ��
        bool init();

        // ������ ���� ��û
        bool connectToServer();

        // ������ �۽�
        bool sendByteData(const char* data, int length);

        // ���� ����
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