

#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "ScenarioParser.hpp"
//#include "UdpMuticast.hpp"
//#include "MissileController.h"
#include <mutex>
#include <string>


class UdpSender1;            // 전방 선언만!
class MissileController;     // 전방 선언만!

class Missile: public std::enable_shared_from_this <Missile> {
public:
    std::string MissileId;
    uint32_t MissileState;
    Location MissileLoc;
  
    // 미사일 생성하자마자 -> 바로 쓰레드 돌리기 시작한다!!
    Missile()
        :MissileId("MSS-000"), MissileState(1), MissileLoc({11.0, 2.0, 3.0}) {
    }

    //std::shared_ptr<MissileController> controller;
    //std::shared_ptr<UdpSender1> sender;

    //// 초기화하는 모듈
    //void init(std::shared_ptr<UdpSender1> s,std::shared_ptr<MissileController> c) {
    //    sender = std::move(s);
    //    controller = std::move(c);
    //    controller->setMissile(shared_from_this());

    //}
    //// 시작하는 모듈
    //void start(float speed) {
    //    if (sender) sender->start();
    //    if (controller) controller->start(speed);
    //}
    //void stop() {
    //    if (sender) sender->close();
    //    if (controller) controller->stop();
    //}

    // 여기안에 맴버함수로 미사일이 해야할 행동들 즉, 모듈들을 끌어와서 여기 넣어주면 된다.
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