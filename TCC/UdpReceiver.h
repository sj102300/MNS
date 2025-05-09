// UdpMulticastReceiver.h
#pragma once

#include <string>
#include <netinet/in.h>

class UdpMulticastReceiver {
public:
    UdpMulticastReceiver(const std::string& groupIp, int port);
    ~UdpMulticastReceiver();

    // ���� �ʱ�ȭ �� ��Ƽĳ��Ʈ �׷� ����
    bool init();

    // ������ ����
    std::string receive();

    // ���� ����
    void close();

private:
    int sockfd_;
    std::string groupIp_;
    int port_;
    struct sockaddr_in localAddr_;
    struct ip_mreq mreq_;
    static constexpr int BUFFER_SIZE = 1024;
};
