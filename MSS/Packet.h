#pragma once
#include "ScenarioParser.h"

struct MissilePacket {
    char MissileId[8];        
    unsigned int MissileState;
    Location MissileLoc;
};

#pragma pack(push, 1)
struct LaunchPacket {
    unsigned int EvenetCode;
    char EventId[20];
    char MissileId[8];
    char AirCraftId[8];
    Location ImpactPoint;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct InterceptResultPacket{
    unsigned int EvenetCode;
    unsigned int BodyLength;
    char EventId[20];
    char MissileId[8];
    char AirCraftId[8];
};
#pragma pack(pop)

#pragma pack(push,1)
struct EDPacket {
    unsigned int EventCode;
    unsigned int BodyLength;
    char EventId[20];
    char MissileId[8];
};
#pragma pack(pop)

#pragma pack(push,1)
struct OrderPacket {
    unsigned int EventCode;
    unsigned int BodyLength;
    char EventId[20];
    char AtsId[8];
    char MissileId[8];
    Location ImpactPoint;
};
#pragma pack(pop)