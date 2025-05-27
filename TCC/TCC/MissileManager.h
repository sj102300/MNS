#pragma once
#include <vector>
#include <string>
#include "Missile.h"

class MissileManager {
private:
    std::vector<Missile*> missiles_;
    void addMissile(Missile* missile);
    Missile* selectMissile(const std::string& id);
    void updateMissile(const std::string& id, TCC::MissileStatus newStatus);
    void checkMissileStatus();
    bool isExistMissile(std::string& missileId);

public:
    ~MissileManager();
};
