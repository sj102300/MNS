#pragma once

#include <string>
#include "Packet.h"  // Location, Velocity ����ü�� ���⿡ �ִٰ� ����

class Aircraft {
public:
    Aircraft(const std::string& id, const Location& location);

    void update(const Location& newLocation);

    const std::string& getId() const;
    const Location& getLocation() const;
    const Velocity& getVelocity() const;

private:
    std::string id_;       // �װ��� �ĺ���
    Location location_;    // ���� ��ġ
    Velocity velocity_;    // �ӵ� (���� + ũ��)
};