#include "Missile.h"

Missile::Missile(std::string id, TCC::Position pos_, int status, TCC::Position impact) : missileId_(id), position_(pos_), status_(static_cast<TCC::MissileStatus>(status)) {
	// 초기화 코드
	status_ = TCC::MissileStatus::Wait; // 대기 상태로 초기화
}

namespace {
	
}

void Missile::updateStatus(TCC::MissileStatus newStatus) {
	status_ = newStatus;
}

std::string Missile::getId() const {
	return missileId_;
}

TCC::MissileStatus Missile::getStatus() const {
	return status_;
}

TCC::Position Missile::getPosition() const {
	return position_;
}

TCC::Position Missile::getImpactPoint() const {
	return impactPoint_;
}