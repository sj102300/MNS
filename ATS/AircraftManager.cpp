#include "AircraftManager.h"
#include <thread>
#include <cmath>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

constexpr double pi = 3.14159265358979;
constexpr double EarthR = 6371.0;

namespace ats {

    static double toRadians(double degree) {
        return degree * (pi / 180.0);
    }

    double haversine(double lat1, double lon1, double lat2, double lon2) {
        lat1 = toRadians(lat1); lon1 = toRadians(lon1);
        lat2 = toRadians(lat2); lon2 = toRadians(lon2);
        double dLat = lat2 - lat1;
        double dLon = lon2 - lon1;
        double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
            std::cos(lat1) * std::cos(lat2) *
            std::sin(dLon / 2) * std::sin(dLon / 2);
        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        return EarthR * c;
    }

    std::vector<double> makeDirection(std::pair<double, double> from, std::pair<double, double> to) {
        double dist = haversine(from.first, from.second, to.first, to.second);
        double step = 0.1;  // 이동 단위 (100ms당 100m 라서)
        int count = std::max(1, static_cast<int>(dist / step));
        double dx = (to.first - from.first) / count;
        double dy = (to.second - from.second) / count;
        return { dx, dy };
    }

    std::pair<double, double> movePoint(std::pair<double, double> current, double dx, double dy) {
        return { current.first + dx, current.second + dy };
    }

    bool reachedDestination(std::pair<double, double> current, std::pair<double, double> dest) {
        return haversine(current.first, current.second, dest.first, dest.second) <= 0.05;
    }

    void sendAircraftInfo(SOCKET udpSocket, const sockaddr_in& addr, const AircraftInfo& info) {
        char buffer[41]{};
        uint32_t eventCode = 1001;
        uint32_t bodyLength = 33;
        double lat = info.currentPoint.first;
        double lon = info.currentPoint.second;
        double alt = 10.0;

        memcpy(buffer, &eventCode, 4);
        memcpy(buffer + 4, &bodyLength, 4);
        memcpy(buffer + 8, info.id.c_str(), 8);
        memcpy(buffer + 16, &lat, sizeof(double));
        memcpy(buffer + 24, &lon, sizeof(double));
        memcpy(buffer + 32, &alt, sizeof(double));
        buffer[40] = info.IFF;

        sendto(udpSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&addr, sizeof(addr));
    }

    void runAircraftManagerThread(std::vector<AircraftInfo> aircraftList, ShootDownThread* shootDownThread) {
        std::thread([aircraftList = std::move(aircraftList), shootDownThread]() mutable {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            inet_pton(AF_INET, "239.0.0.1", &addr.sin_addr);
            addr.sin_port = htons(9000);

            std::vector<std::vector<double>> directions;
            for (auto& ac : aircraftList) {
                directions.push_back(makeDirection(ac.startPoint, ac.finishPoint));
            }

            while (true) {
                bool allArrived = true;
                for (size_t i = 0; i < aircraftList.size(); ++i) {
                    auto& ac = aircraftList[i];
                    auto& dir = directions[i];

                    if (!reachedDestination(ac.currentPoint, ac.finishPoint)) {
                        allArrived = false;
                        ac.currentPoint = movePoint(ac.currentPoint, dir[0], dir[1]);

                        sendAircraftInfo(udpSocket, addr, ac);

                        if (shootDownThread) {
                            shootDownThread->enqueue({ ac.id, ac.currentPoint, ac.IFF });
                        }
                    }
                }

                if (allArrived) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            closesocket(udpSocket);
            WSACleanup();
            }).detach();
    }

}  // namespace ats
