#pragma once

#include "UdpSender.h"

class IMissileSender {
public:
    virtual void sendMissileData() = 0;
    virtual ~IMissileSender() = default;
};

class MissileSender : public IMissileSender {
public:
    void sendMissileData() override;
    void makeMissilePacket();

private:
    class TCC::UdpSender* udpSender;
};
