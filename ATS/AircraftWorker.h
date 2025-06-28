#pragma once

#include "ShootDownThread.h"
#include <string>
#include <thread>
#include <atomic>
#include <utility>
#include <vector>
#define NOMINMAX
#include <winsock2.h>

namespace ats {

    struct AircraftInfo {
        std::string id;
        std::pair<double, double> startPoint;
        std::pair<double, double> finishPoint;
        std::pair<double, double> currentPoint;
        char IFF;

        AircraftInfo() = default;

        AircraftInfo(const std::string& id_,
            std::pair<double, double> start_,
            std::pair<double, double> finish_,
            std::pair<double, double> current_,
            char iff_)
            : id(id_), startPoint(start_), finishPoint(finish_), currentPoint(current_), IFF(iff_) {
        }
    };

    class AircraftWorker {
    public:
        AircraftWorker(AircraftInfo info, ShootDownThread* shootDownThread);
        void stop();
        void operator()();

    private:
        AircraftInfo info_;
        bool running_;
        ShootDownThread* shootDownThread_;  // 격추 판단 요청용

        std::vector<double> makeDirection(std::pair<double, double> from, std::pair<double, double> to);
        std::pair<double, double> movePoint(std::pair<double, double> current, double dx, double dy);
        bool reachedDestination(std::pair<double, double> current, std::pair<double, double> dest);
        void sendAircraftInfo(SOCKET udpSocket, const sockaddr_in& addr);
    };

} // namespace ats
