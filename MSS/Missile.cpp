#pragma once

#include "Missile.h"
#include "UdpMuticast.h"
#include "MissileController.h"

Missile::Missile()
    : MissileId("MSS-000"), MissileState(0), MissileLoc({ 0.0, 0.0, 10 }) {
}

void Missile::init(std::shared_ptr<UdpMulticast> s, std::shared_ptr<MissileController> c) {
    sender = std::move(s);
    sender->setMissile(shared_from_this());
    controller = std::move(c);
    controller->setMissile(shared_from_this());
    controller->setTarget({0,0,0});
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