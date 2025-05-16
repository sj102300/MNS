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

        // �ʱ�ȭ: ���� ���� ��
        bool init();

        // ������ ���� ��û
        bool connectToServer();

        // ������ �۽�
        virtual bool sendData(const char* data, int length) = 0;

        // ��û ����
        virtual void getResponse() = 0;

        // ������ ����. Ŭ���̾�Ʈ ����
        virtual void start() = 0;

        // ���� ����
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