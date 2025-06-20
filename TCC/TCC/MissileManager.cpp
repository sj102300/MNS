// MissileManager.cpp
#include "MissileManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "UdpSender.h"
#include "UdpMulticastReceiver.h"

MissileManager::MissileManager(TCC::UdpSender* sender, EngagementManager* engagement) : udpSender_(sender), engagementManager_(engagement) {

#if DEBUG
    for (int i = 1; i <= 6; ++i) { // 6대 생성, 100대로 늘리려면 6 -> 100으로 변경
        std::ostringstream oss;
        oss << "MSS-" << std::setw(3) << std::setfill('0') << i;
        std::string id = oss.str();

        TCC::Position pos = { 0.0, 0.0, 0.0 }; // 초기 위치
        int status = Missile::MissileStatus::Wait; // 초기 상태
        TCC::Position impact = { 0.0, 0.0, 0.0 }; // 초기 impactPoint

        Missile* missile = new Missile(id, pos, status, impact);
        addMissile(missile);
    }
#endif

    for (int i = 100;i <= 105;++i) {
        std::string id = "MSS-" + std::to_string(i);
        TCC::Position pos = { 0.0, 0.0, 0.0 }; // 초기 위치
        int status = Missile::MissileStatus::Wait; // 초기 상태
        TCC::Position impact = { 0.0, 0.0, 0.0 }; // 초기 impactPoint

        Missile* missile = new Missile(id, pos, status, impact);
        addMissile(missile);
    }

#if DEBUG
    TCC::Position pos = { 0.0, 0.0, 0.0 }; // 초기 위치
    int status = Missile::MissileStatus::Wait; // 초기 상태
    TCC::Position impact = { 0.0, 0.0, 0.0 }; // 초기 impactPoint
    Missile* missile = new Missile("MSS-100", pos, status, impact);
    addMissile(missile);
#endif

    std::cout << "미사일 객체 6개 생성 완료" << "\n";
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
        if (m->getMissileStatus() == Missile::MissileStatus::LaunchRequest) {
            if (newStatus != Missile::MissileStatus::Fly) {
                return;     //발사 요청 중일때 비행 상태로 변경된거 외에는 상태 업데이트 하면안됨
            }
        }
        m->updateStatus(newStatus);
        //std::cout << "Updated missile " << id << " to status " << (unsigned int)newStatus << "\n";
    }
    else {
        //std::cout << "Missile " << id << " not found\n";
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
            //std::cout << u8"미사일 데이터 전송 성공\n";
        }
        else {
            std::cout << u8"미사일 데이터 전송 실패\n";
        }
    }
    
    std::string missileId(msg.missileId, 7);
    for (auto* m : missiles_) {
        if (m->checkId(missileId)) {
            m->updatePosition(msg.location_);
            break;
        }
        //std::cout << "Missile ID: " << m->getId()/*
        //    << ", Status: " << m->getStatus() << "\n";*/
    }

    //자폭한 경우
    if (msg.status_ == Missile::MissileStatus::SelfDestroyed) {
        engagementManager_->handleMissileDestroyed(missileId, EngagementManager::DestroyType::SelfDestroy);
    }

  //  if (msg.status_ == 2) { // 2: 격추 성공
		//std::string missileId(msg.missileId, 8);
  //      if (engagementManager_) {
  //          engagementManager_->isHitTarget(missileId);

  //      }
  //  }
	updateMissileStatus(msg.missileId, Missile::MissileStatus(msg.status_));
}

std::string MissileManager::findAvailableMissile() {
    //std::cout << "findAvailableMissile() called\n";
    std::string missileId;
	for (auto m : missiles_) {
		if (m->isAvailable(0, missileId)) { // 사용가능한 미사일
			//std::cout << "Found available missile: " << m->checkId("MSS-100") << "\n";
            m->updateStatus(Missile::MissileStatus::LaunchRequest);
            
            std::thread([m]() {
                //1초후에 실행
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (m->getMissileStatus() != Missile::MissileStatus::Fly) {
                    //발사 요청 실패한것
                    m->updateStatus(Missile::MissileStatus::Wait);
                }
                }).detach();

			return missileId;
		}
	}
	std::cout << "No available missile found\n";
	return ""; // 사용 가능한 미사일이 없을 경우 빈 문자열 반환
}

MissileManager::~MissileManager() {
    for (Missile* missile : missiles_) {
        delete missile;
    }
    missiles_.clear(); 
}
