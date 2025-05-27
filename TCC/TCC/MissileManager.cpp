// MissileManager.cpp
#include "MissileManager.h"
#include <iostream>

MissileManager::~MissileManager() {
    for (auto m : missiles_) {
        delete m;
    }
}

void MissileManager::addMissile(Missile* missile) {
    missiles_.push_back(missile);
}

Missile* MissileManager::selectMissile(const std::string& id) {
    for (auto* m : missiles_) {
        if (m->getId() == id) return m;
    }
    return nullptr;
}

void MissileManager::updateMissile(const std::string& id, TCC::MissileStatus newStatus) {
    Missile* m = selectMissile(id);
    if (m) {
        //m->updateStatus(newStatus);
        std::cout << "Updated missile " << id << " to status " << (unsigned int)newStatus << "\n";
    }
    else {
        std::cout << "Missile " << id << " not found\n";
    }
}

void MissileManager::checkMissileStatus() {
    for (auto* m : missiles_) {
        //std::cout << "Missile ID: " << m->getId()/*
        //    << ", Status: " << m->getStatus() << "\n";*/
    }
}
