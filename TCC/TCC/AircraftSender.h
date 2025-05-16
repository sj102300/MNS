#pragma once

#include "UdpSender.h"
#include "share.h"
#include <queue>
#include <thread>
#include <mutex>

class IAircraftSender {
public:
    typedef struct _new_aircraft_data_with_IP_data {
        std::string aircraftId_;
        TCC::Position location_;
        bool isEnemy_;
        unsigned int engagementStatus_;
        TCC::Position impactPoint_;
    } NewAircraftWithIP;
    virtual void sendAircraftData() = 0;
    virtual void pushSendQueue(NewAircraftWithIP& newAircraftWithIp) = 0;

protected:
	virtual bool popSendQueue(NewAircraftWithIP& newAircraftWithIp) = 0;
};

class AircraftSender : public TCC::UdpSender, public IAircraftSender {
public:
    AircraftSender(const std::string& ip_address, int port);
    void sendAircraftData() override;
    void pushSendQueue(NewAircraftWithIP& newAircraftWithIp) override;
    bool popSendQueue(NewAircraftWithIP& newAircraftWithIp) override;

private:
    void writeToBuffer(NewAircraftWithIP& msg);
    void sendAircraftMessage();
    std::mutex mtx_;
    std::queue<NewAircraftWithIP> sendQueue;
    std::thread sendThread_;
    char buffer[64];
    int len;
};
