#pragma once

#include "ScenarioManager.h"

class ScenarioInfoPrinter {
public:
    void printInfo(const ScenarioManager& manager) const;
    void printBattery(const ScenarioManager& manager) const;
    void printAircraftList(const ScenarioManager& manager) const;
    void printAll(const ScenarioManager& manager) const;  // 전체 출력
};
