#pragma once

#include "Missile.h"
#include "ShootDown.h"
#include "sender.h"
#include "AircraftCoordinate.h"
#include "UdpMulticastReceiver.h"
#include <string>
#include <thread>
#include <atomic>
#include <utility>

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
        AircraftWorker(AircraftInfo info);
        void operator()();
        void stop();

    private:
        AircraftInfo info_;
        std::atomic<bool> running_;
    };

} // namespace ats
