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


// �̻��� ���� ����� ���� �װ��� ���� 
#pragma pack(push,1)
struct AirCraftPacket {
    unsigned int EventCode;
    unsigned int BodyLength;
    char AtsId[8];
    Location AtsLoc;
    char AtsIdentify;
};
#pragma pack(pop)

// �ӵ��� ����ϱ����� ������ �浵 ��ȭ��??
#pragma pack(push,1)
struct Velocity {
    double dLatitudePerSec;
    double dLongitudePerSec;
};
#pragma pack(pop)