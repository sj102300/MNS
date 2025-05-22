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

    enum class MissileStatus : unsigned int {
        Wait = 0,   // 대기(발사 전)
        Fly = 1,      // 비행(표적을 향해 비행 중)
        Splashed = 2,        // 격추(미사일이 표적 격추)
        EmergencyDestroyed = 3,        // 비상폭파(사용자가 비상폭파 명령 내림)
        SelfDestroyed = 4        // 자폭 (IP지났을 때 자동 폭파)
    };

    bool isValidAircraftId(const char* input);
    bool isValidMissile(const char* input);
    bool isValidCommand(const char* input);
}
