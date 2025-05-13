#pragma once

#include "TcpSender.h"
#include "share.h"
#include <queue>
#include <mutex>

class IAircraftSender {
public:
	typedef struct aircraft_send_message {
        char aircraftId_[8]; //8ÀÚ
        TCC::Position location_;
        unsigned int isEnemy_;
        unsigned int engagementStatus_;
        TCC::Position impactPoint_;
	} AircraftMSG;
    
	virtual void sendAircraftData() = 0;
    virtual void pushSendQueue(IAircraftSender::AircraftMSG &msg) = 0;
    virtual bool popSendQueue(IAircraftSender::AircraftMSG& msg) = 0;
};

class AircraftSender : public TCC::TcpSender, public IAircraftSender {
public:
    AircraftSender(const std::string& ip_address, int port);
    void sendAircraftData() override;
    void writeToBuffer(IAircraftSender::AircraftMSG &msg);

private:
    void pushSendQueue(IAircraftSender::AircraftMSG& msg);
    bool popSendQueue(IAircraftSender::AircraftMSG& msg);
    std::mutex mtx_;
    std::queue<IAircraftSender::AircraftMSG> sendQueue;
    char buffer[64];
    int len;
};
