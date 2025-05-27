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
        bool isValidPosition();          // ��� ���� ����
        bool isZeroVector();
    } Position;

    enum class MissileStatus : unsigned int {
        Wait = 0,   // ���(�߻� ��)
        Fly = 1,      // ����(ǥ���� ���� ���� ��)
        Splashed = 2,        // ����(�̻����� ǥ�� ����)
        EmergencyDestroyed = 3,        // �������(����ڰ� ������� ��� ����)
        SelfDestroyed = 4        // ���� (IP������ �� �ڵ� ����)
    };

    bool isValidAircraftId(const char* input);
    bool isValidMissile(const char* input);
    bool isValidCommand(const char* input);
}
