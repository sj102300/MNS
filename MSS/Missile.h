#pragma once
#include "Packet.h"
#include <thread>

class MissileController;
class UdpMulticast;

class Missile : public std::enable_shared_from_this <Missile> {
public:
    std::string MissileId;
    unsigned int MissileState;
    Location MissileLoc;

    // �̻��� �������ڸ��� -> �ٷ� ������ ������ �����Ѵ�!!
    Missile();

    void setMissileId(const std::string& id);

    void setState(unsigned int state);

    void setTargetLocation(const Location& loc);

    // �ʱ�ȭ�ϴ� ���
    void init(std::shared_ptr<UdpMulticast> s, std::shared_ptr<MissileController> c);

    // �����ϴ� ���
    void start(float speed);

    void stop();
private:
    std::shared_ptr<MissileController> controller;
    std::shared_ptr<UdpMulticast> sender;

};