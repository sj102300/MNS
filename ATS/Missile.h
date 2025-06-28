#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <mutex>

#pragma pack(push, 1)
struct Missile {
    uint32_t eventCode;   // 1001 or 1002
    uint32_t bodyLength;  // fixed: 33
    char missileId[8];    // e.g. "ATS-0001"
    uint32_t status;
    double latitude;      // -90 ~ 90
    double longitude;     // -180 ~ 180
    double altitude;      // 0 ~ 10 (km)
};
#pragma pack(pop) // 정렬 없이 그대로 41바이트 구조체 생성

struct ParsedMissileData {
    std::string missileId;
    double latitude;
    double longitude;
    double altitude;
    uint32_t eventCode;
};

extern std::unordered_map<std::string, ParsedMissileData> globalMissiles;
extern std::mutex missileMtx;