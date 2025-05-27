#pragma once

namespace TCC {

    typedef struct _position {
        double latitude_;
        double longitude_;
        double altitude_;

        bool isValidPosition(double latitude, double longitude, double altitude);
        bool isValidLatitude(double latitude);
        bool isValidLongitude(double longitude);
        bool isValidAltitude(double altitude);
        bool isValidPosition();          // 멤버 변수 기준
        bool isZeroVector();
    } Position;

    bool isValidAircraftId(const char* input);
    bool isValidMissileId(const char* input);
    bool isValidCommand(const char* input);
}
