#include "Missile.h"

Missile::Missile(std::string id, TCC::Position pos_, int status, TCC::Position impact) : missileId_(id), position_(pos_), status_(Missile::MissileStatus(status)) {
	// 초기화 코드
	status_ = MissileStatus::Wait; // 대기 상태로 초기화
}

Missile::~Missile() {}

namespace {
	
}

void Missile::updateStatus(MissileStatus newStatus) {
	status_ = newStatus;
}

unsigned int Missile::getMissileStatus() {
	return status_;
}

bool Missile::getCurLocation(TCC::Position& loc) {
	loc = position_;
	return true;
}

bool Missile::checkId(std::string id) {
	if (id == missileId_) {
		return true;
	}
	return false;
}

bool Missile::isAvailable(unsigned int status, std::string& missileId) {
	if (status_ == status) {
		missileId = missileId_;
		return true;
	}
	return false;
}

