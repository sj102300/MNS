

#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "ScenarioParser.hpp"
//#include "UdpMuticast.hpp"
//#include "MissileController.h"
#include <mutex>
#include <string>


class UdpSender1;            // ���� ����!
class MissileController;     // ���� ����!

class Missile: public std::enable_shared_from_this <Missile> {
public:
    std::string MissileId;
    uint32_t MissileState;
    Location MissileLoc;
  
    // �̻��� �������ڸ��� -> �ٷ� ������ ������ �����Ѵ�!!
    Missile()
        :MissileId("MSS-000"), MissileState(1), MissileLoc({11.0, 2.0, 3.0}) {
    }

    //std::shared_ptr<MissileController> controller;
    //std::shared_ptr<UdpSender1> sender;

    //// �ʱ�ȭ�ϴ� ���
    //void init(std::shared_ptr<UdpSender1> s,std::shared_ptr<MissileController> c) {
    //    sender = std::move(s);
    //    controller = std::move(c);
    //    controller->setMissile(shared_from_this());

    //}
    //// �����ϴ� ���
    //void start(float speed) {
    //    if (sender) sender->start();
    //    if (controller) controller->start(speed);
    //}
    //void stop() {
    //    if (sender) sender->close();
    //    if (controller) controller->stop();
    //}

    // ����ȿ� �ɹ��Լ��� �̻����� �ؾ��� �ൿ�� ��, ������ ����ͼ� ���� �־��ָ� �ȴ�.
};

struct MissilePacket {
    char MissileId[8];        
    uint32_t MissileState;
    Location MissileLoc;
};

#pragma pack(push, 1)
struct LaunchPacket {
    uint32_t EvenetCode;
    char EventId[20];
    char MissileId[8];
    char AirCraftId[8];
    Location ImpactPoint;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct InterceptResultPacket{
    uint32_t EvenetCode;
    char EventId[20];
    char MissileId[8];
    char AirCraftId[8];
};
#pragma pack(pop)

#pragma pack(push,1)
struct EDPacket {
    uint32_t EventCode;
    char EventId[20];
    char MissileId[8];
};
#pragma pack(pop)