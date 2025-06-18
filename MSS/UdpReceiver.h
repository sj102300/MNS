#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <gtest/gtest.h>
#include "IReceiver.h" // 이 부분 테스팅 위해 추가
#include "Missile.h"
#include "Aircraft.h"

class UdpReceiver : public IReceiver {
public:
    UdpReceiver();

    // 멀티캐스트 주소와 포트를 지정해 초기화
    virtual bool init(const std::string& multicast_address, int port);

    // 수신 루프 시작
    virtual void start();

    // 수신 루프 내용
    virtual void run();

    // 종료 및 자원 해제
    virtual void close();

    // 미사일 설정
    void setMissileMap(const std::unordered_map<std::string, std::shared_ptr<Missile>>& map); // 6개의 미사일 객체를 관리하기 위함


private:
    SOCKET sock_;
    sockaddr_in localAddr_;
    ip_mreq mreq_;  // 멀티캐스트 그룹 정보
    std::atomic<bool> running_;
    std::thread recvThread_;
    int port_;
    std::shared_ptr<Missile> missile_;
    std::unordered_map<std::string, std::shared_ptr<Missile>> missile_map_;
    std::shared_ptr<Aircraft> aircraft_;
    std::unordered_map <std::string, std::shared_ptr<Aircraft>> Aircraft_map_;
};
