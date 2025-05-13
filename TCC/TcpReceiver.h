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

        //recv ������ ���۽� �ѹ��� ȣ��
        virtual void start() = 0;

        virtual void receive() = 0;

        // ���� ó��
        virtual void response() = 0;

        // ���� �ڵ鸵 ���� ����
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