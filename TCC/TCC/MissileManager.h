#pragma once
#include <vector>
#include <string>
#include "Missile.h"
#include "UdpSender.h"
#include "UdpMulticastReceiver.h"

class MissileManager {
private:
    std::vector<Missile*> missiles_;
    TCC::UdpSender* udpSender_;

    void addMissile(Missile* missile);
    Missile* selectMissile(const std::string& id);
    void updateMissileStatus(const std::string& id, Missile::MissileStatus newStatus);
    void checkMissileStatus();
    bool isExistMissile(std::string& missileId);
    
public:
	MissileManager(TCC::UdpSender* udpSender); // »ý¼ºÀÚ
	void echoMissileData(TCC::UdpMulticastReceiver::MissileMSG& data); 
    std::string findAvailableMissile(); 

    ~MissileManager();
};
