#pragma once

#include<unordered_set>
#include <string>
#include<mutex>
#include <iostream>

class DestroyedAircraftsTracker {
public:
    std::unordered_set<std::string> destroyedAircrafts;  // �ı��� �װ��� ID ���
    std::mutex destroyedAircraftsMutex;  // �ı��� �װ��� ��Ͽ� ���� ���ؽ�
    void findNewDestroyedAircraft(std::string& aircraftId);
    bool isDestroyedAircraft(std::string& aircraftId);
    void clearDestroyedAircrafts();
};
