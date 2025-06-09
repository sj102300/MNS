#pragma once

#pragma pack(push,1)
struct Location {
    double latitude;
    double longitude;
    double altitude;
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