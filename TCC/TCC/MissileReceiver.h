#pragma once
#include "share.h"
#include <string>
#include "UdpMulticastReceiver.h"

class IMissileReceiver {
private:

public:
    typedef struct _missile_message {
        char missileId[8]; // 8자
        unsigned int missileStatus; // 미사일 상태 4bit
        TCC::Position position; // 미사일 위치(위도:8byte, 경도:8byte, 고도:8byte)
    } MissileMSG;

    virtual void receiveMissileData() = 0;
    virtual ~IMissileReceiver() = default;
};


class MissileReceiver : public TCC::UdpMulticastReceiver, public IMissileReceiver {
public:
    MissileReceiver(const std::string& multicastIp, int port);
    //void receive() override;
	~MissileReceiver();
    void parseMissileMessage();

private:
    // Dependency Injection 방식
    class UdpMulticastReceiver* udpReceiver;
};
