#pragma once

#include <string>
#include <vector>

#pragma pack(push, 1)
struct Location {
    double latitude;
    double longitude;
    double altitude;
};
#pragma pack(pop)

struct MissilePacket {
    unsigned int EventCode;
    unsigned int BodyLength;
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


// 미사일 유도 기능을 위한 항공기 정보 
#pragma pack(push,1)
struct AirCraftPacket {
    unsigned int EventCode;
    unsigned int BodyLength;
    char AtsId[8];
    Location AtsLoc;
    char AtsIdentify;
};
#pragma pack(pop)

// 속도를 계산하기위한 위도와 경도 변화량??
#pragma pack(push,1)
struct Velocity {
    double dLatitudePerSec;
    double dLongitudePerSec;
};
#pragma pack(pop)