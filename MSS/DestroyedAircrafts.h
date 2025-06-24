#pragma once

#include<unordered_set>
#include <string>
#include<mutex>
#include <iostream>

class DestroyedAircraftsTracker {
public:
    std::unordered_set<std::string> destroyedAircrafts;  // 파괴된 항공기 ID 목록
    std::mutex destroyedAircraftsMutex;  // 파괴된 항공기 목록에 대한 뮤텍스
    void findNewDestroyedAircraft(std::string& aircraftId);
    bool isDestroyedAircraft(std::string& aircraftId);
    void clearDestroyedAircrafts();
};
