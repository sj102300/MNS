#pragma once
#include <vector>
#include <string>
#include "Missile.h"
#include "UdpSender.h"
#include "UdpMulticastReceiver.h"
#include "EngagementManager.h"

class MissileManager {
private:
    std::vector<Missile*> missiles_;
    TCC::UdpSender* udpSender_;
    EngagementManager* engagementManager_;

    void addMissile(Missile* missile);
    void updateMissileStatus(const std::string& id, Missile::MissileStatus newStatus);
    void checkMissileStatus();
    //bool isExistMissile(std::string& missileId);
    
public:
	MissileManager(TCC::UdpSender* udpSender, EngagementManager* engagementManager); // »ý¼ºÀÚ
	void echoMissileData(TCC::UdpMulticastReceiver::MissileMSG& data);
    Missile* selectMissile(const std::string& id);
    std::string findAvailableMissile(); 

    ~MissileManager();
};
