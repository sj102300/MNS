#pragma once
#include "ScenarioParser.hpp"

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