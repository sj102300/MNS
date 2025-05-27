#pragma once
#include "share.h"
#include <string>
#include "UdpMulticastReceiver.h"

class IMissileReceiver {
private:

public:
    typedef struct _missile_message {
        char missileId[8]; // 8��
        unsigned int missileStatus; // �̻��� ���� 4bit
        TCC::Position position; // �̻��� ��ġ(����:8byte, �浵:8byte, ��:8byte)
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
    // Dependency Injection ���
    class UdpMulticastReceiver* udpReceiver;
};
