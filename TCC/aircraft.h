#pragma once

#include <string>
#include "share.h"

class AirCraft {
public:
    bool isEnemy();
    TCC::Position getImpactPoint();
    bool canEngageTarget();

private:
    std::string aircraftId;
    TCC::Position pos;
    char friend_or_enemy;
    unsigned int status;
};
