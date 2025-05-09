// UdpMulticastReceiver.h
#pragma once

#include <string>
#include <netinet/in.h>

class UdpMulticastReceiver {
public:
    UdpMulticastReceiver(const std::string& groupIp, int port);
    ~UdpMulticastReceiver();

    // 소켓 초기화 및 멀티캐스트 그룹 가입
    bool init();

    // 데이터 수신
    std::string receive();

    // 소켓 종료
    void close();

private:
    int sockfd_;
    std::string groupIp_;
    int port_;
    struct sockaddr_in localAddr_;
    struct ip_mreq mreq_;
    static constexpr int BUFFER_SIZE = 1024;
};
