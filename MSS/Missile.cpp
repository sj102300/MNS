#pragma once

#include "Missile.h"
#include "UdpMuticast.hpp"
#include "MissileController.h"

Missile::Missile()
    : MissileId("MSS-000"), MissileState(1), MissileLoc({ 1.0, 2.0, 3.0 }) {
}

void Missile::init(std::shared_ptr<UdpMulticast> s, std::shared_ptr<MissileController> c) {
    sender = std::move(s);
    sender->setMissile(shared_from_this());
    controller = std::move(c);
    controller->setMissile(shared_from_this());
    controller->setTarget({100,200,20});
}

void Missile::start(float speed) {
    if (sender) { sender->start(); }
    if (controller) controller->start(speed);
}

void Missile::stop() {
    if (sender) sender->close();
    if (controller) controller->stop();
}