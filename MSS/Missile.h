#pragma once
#include "Packet.h"
#include <thread>
#include <mutex>

#include "DestroyedAircrafts.h"

class MissileController;
class UdpMulticast;

class Missile : public std::enable_shared_from_this <Missile> {
public:
    std::string MissileId;
    unsigned int MissileState;
    Location MissileLoc;
    std::string TargetAircraftId;  // [추가] 타겟 항공기 ID
    std::shared_ptr<MissileController> getController();  // ← 이 줄 추가
	std::mutex mtx;  // 쓰레드 안전을 위한 뮤텍스

    // 미사일 생성하자마자 -> 바로 쓰레드 돌리기 시작한다!!
    Missile();

    void setMissileId(const std::string& id);

    void setState(unsigned int state);

    void setLoc(Location& loc);

    void setTargetLocation(const Location& loc);

    // 초기화하는 모듈
    void init(std::shared_ptr<UdpMulticast> s, std::shared_ptr<MissileController> c);

    // [추가] 타겟 항공기 ID 설정
    void setTargetAircraftId(const std::string& aircraftId);
    const std::string& getTargetAircraftId() const;

    // 시작하는 모듈
    void start(float speed);

    void stop();
private:
    std::shared_ptr<MissileController> controller;
    std::shared_ptr<UdpMulticast> sender;
};