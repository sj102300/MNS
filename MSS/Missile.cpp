#pragma once

#include "Missile.h"
#include "UdpMuticast.h"
#include "MissileController.h"
#include "DestroyedAircrafts.h"

Missile::Missile()
    : MissileId("MSS-000"), MissileState(0), MissileLoc({ 0.0, 0.0, 10 }) {
}

void Missile::init(std::shared_ptr<UdpMulticast> s, std::shared_ptr<MissileController> c) {
    sender = std::move(s);
    sender->setMissile(shared_from_this());
    controller = std::move(c);
    controller->setMissile(shared_from_this());
    controller->setTarget({0,0,0});

    // [추가] 타겟 항공기 ID가 지정되어 있다면 전달
    if (!TargetAircraftId.empty()) {
        controller->setTargetAircraftId(TargetAircraftId);
    }
}
void Missile::setLoc(Location& loc) {
    MissileLoc.altitude = loc.altitude;
    MissileLoc.latitude = loc.latitude;
    MissileLoc.longitude = loc.longitude;
}

void Missile::setMissileId(const std::string& id) {
    MissileId = id;
}

void Missile::setState(uint32_t state) {
	std::lock_guard<std::mutex> lock(mtx);  // 쓰레드 안전을 위한 뮤텍스 사용
    MissileState = state;
}
void Missile::setTargetLocation(const Location& loc) {
    if (controller) {
        controller->setTarget(loc);
    }
}
void Missile::start(float speed) {
    if (sender) { sender->start(); }
    if (controller) controller->start(speed);
}

void Missile::stop() {
    if (sender) sender->close();
    if (controller) controller->stop();
}

// [추가] 타겟 항공기 ID 설정 함수 정의
void Missile::setTargetAircraftId(const std::string& aircraftId) {
    TargetAircraftId = aircraftId;
}

const std::string& Missile::getTargetAircraftId() const {
    return TargetAircraftId;
}

std::shared_ptr<MissileController> Missile::getController() {
    return controller;
}
