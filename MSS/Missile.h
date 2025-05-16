#pragma once
#include "Packet.hpp"
#include <thread>

class MissileController;
class UdpMulticast;

class Missile : public std::enable_shared_from_this <Missile> {
public:
    std::string MissileId;
    uint32_t MissileState;
    Location MissileLoc;

    // 미사일 생성하자마자 -> 바로 쓰레드 돌리기 시작한다!!
    Missile();

    // 초기화하는 모듈
    void init(std::shared_ptr<UdpMulticast> s, std::shared_ptr<MissileController> c);

    // 시작하는 모듈
    void start(float speed);

    void stop();
private:
    std::shared_ptr<MissileController> controller;
    std::shared_ptr<UdpMulticast> sender;

};