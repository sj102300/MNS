#pragma once

#include "HttpClient.h"

namespace sm {
    class ScenarioInfoPrinter {
    public:
        void printInfo(const HttpClient& client) const;
        void printBattery(const HttpClient& client) const;
        void printAircraftList(const HttpClient& client) const;
        void printAll(const HttpClient& client) const;
    };
}
