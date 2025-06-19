#pragma once

#include <string>
#include "Packet.h"  // Location, Velocity 구조체가 여기에 있다고 가정

class Aircraft {
public:
    Aircraft(const std::string& id, const Location& location);

    void update(const Location& newLocation);

    const std::string& getId() const;
    const Location& getLocation() const;
    const Velocity& getVelocity() const;

private:
    std::string id_;       // 항공기 식별자
    Location location_;    // 현재 위치
    Velocity velocity_;    // 속도 (방향 + 크기)
};