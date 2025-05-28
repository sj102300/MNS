#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace mfr {

#pragma pack(push, 1)
    struct AircraftPacket {
        uint32_t eventCode;   // 1001 or 1002
        uint32_t bodyLength;  // fixed: 33
        char aircraftId[8];   // e.g. "ATS-0001"
        double latitude;      // -90 ~ 90
        double longitude;     // -180 ~ 180
        double altitude;      // 0 ~ 10 (km)
        char fooFlag;         // 'E' or 'O'
    };
#pragma pack(pop) // ���� ���� �״�� 41����Ʈ ����ü ����

    class MFR {
    public:
        MFR(const std::string& udpIp, int udpPort);
        ~MFR();

        void run(double batteryLat, double batteryLon, double batteryAlt);       // ���� �� �Ǵ� �� �۽�
        void stop();      // ���� ���� ��û

    private:
        SOCKET sock_;
        sockaddr_in sendAddr_;
        std::atomic<bool> stopFlag_ = false;  // ���� �÷���

        bool setupSocket(const std::string& recvIp, int recvPort);
        bool withinRange(double lat, double lon, double alt,
            double batteryLat, double batteryLon, double batteryAlt); // ���̴� �Ǵ�

        static constexpr double maxDetectKm = 300.0;
    };

}
