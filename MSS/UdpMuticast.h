#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "Missile.h"
#include "Isender.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <memory>

#pragma comment(lib, "ws2_32.lib")


class UdpMulticast : public Isender {

public:
     UdpMulticast();

        // 멀티캐스트 대상 주소와 포트를 받아 초기화
     virtual bool init(const std::string& multicast_address, int port);
        // 객체의 맞는 정보를 전달하기 위함
     virtual void setMissile(std::shared_ptr<Missile> m);
        // 전송 루프: 주기적으로 데이터를 보내는 예시
    virtual void start();
        // 전송 루프 내용: 2.5초마다 데이터 전송
    virtual void run();
        // 종료 및 자원 해제
    virtual void close();

    MissilePacket serializeMissile(const Missile& missile);

private:
    SOCKET sock_;
    sockaddr_in destAddr_;  // 멀티캐스트 대상 주소 저장
    int port_;
    bool running_;
    std::thread sendThread_;
    std::shared_ptr<Missile> missile_;
};
