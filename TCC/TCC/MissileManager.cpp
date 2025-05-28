// MissileManager.cpp
#include "MissileManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "UdpSender.h"
#include "UdpMulticastReceiver.h"

MissileManager::MissileManager(TCC::UdpSender* sender, EngagementManager* engagement) : udpSender_(sender), engagementManager_(engagement) {
    for (int i = 1; i <= 6; ++i) { // 6�� ����, 100��� �ø����� 6 -> 100���� ����
        std::ostringstream oss;
        oss << "MSS-" << std::setw(3) << std::setfill('0') << i;
        std::string id = oss.str();

        TCC::Position pos = { 0.0, 0.0, 0.0 }; // �ʱ� ��ġ
        int status = Missile::MissileStatus::Wait; // �ʱ� ����
        TCC::Position impact = { 0.0, 0.0, 0.0 }; // �ʱ� impactPoint

        Missile* missile = new Missile(id, pos, status, impact);
        addMissile(missile);
    }

    std::cout << "�̻��� ��ü 6�� ���� �Ϸ�" << "\n";
}

void MissileManager::addMissile(Missile* missile) {
    missiles_.push_back(missile);
}

Missile* MissileManager::selectMissile(const std::string& id) {
    for (auto* m : missiles_) {
        if (m->checkId(id)) return m;
    }
    return nullptr;
}

void MissileManager::updateMissileStatus(const std::string& id, Missile::MissileStatus newStatus) {
    Missile* m = selectMissile(id);
    if (m) {
        m->updateStatus(newStatus);
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

void MissileManager::echoMissileData(TCC::UdpMulticastReceiver::MissileMSG& msg) {
    if (udpSender_) {
        if (udpSender_->sendMissileData(msg)) {
            std::cout << "�̻��� ������ ���� ����\n";
        }
        else {
            std::cout << "�̻��� ������ ���� ����\n";
        }
    }
    if (msg.status_ == 2) { // 2: ���� ����
		std::string missileId(msg.missileId, 8);
        if (engagementManager_) {
            engagementManager_->isHitTarget(missileId);

        }
    }
	updateMissileStatus(msg.missileId, Missile::MissileStatus(msg.status_));
}

std::string MissileManager::findAvailableMissile() {
    std::string missileId;
	for (auto* m : missiles_) {
		if (m->isAvailable(0, missileId)) { // ��밡���� �̻���
			std::cout << "Found available missile: " << m->checkId("MSS-001") << "\n";
			return missileId;
		}
	}

	std::cout << "No available missile found\n";
	return ""; // ��� ������ �̻����� ���� ��� �� ���ڿ� ��ȯ
}

MissileManager::~MissileManager() {
    //for (auto m : missiles_) {
    //    delete m;
    //}
}
