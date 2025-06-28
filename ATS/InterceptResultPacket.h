// InterceptResultPacket.h
#pragma once

#pragma pack(push, 1)
struct InterceptResultPacket {
    unsigned int EventCode;      // 예: 2003
    unsigned int BodyLength;     // payload 길이
    char EventId[20];            // 예: "SUCCESS"
    char AirCraftId[8];          // 항공기 ID
    char MissileId[8];           // 미사일 ID
};
#pragma pack(pop)
