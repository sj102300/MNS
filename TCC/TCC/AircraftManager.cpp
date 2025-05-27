
#include "AircraftManager.h"
#include "UdpSender.h"
#include "EngagementManager.h"

AircraftManager::AircraftManager() {

}

AircraftManager::~AircraftManager() {
	for (auto& v : aircrafts_) {
		delete v.second;
	}
}

bool AircraftManager::init(TCC::UdpSender* sender, EngagementManager* engagementManager) {
	//engagementManager도 nullptr확인
	if (sender == nullptr) {
		return false;
	}
	engagementManager_ = engagementManager;
	sender_ = sender;
	return true;
}

void AircraftManager::start() {
	workThread_ = std::thread(&AircraftManager::judgeEngagable, this);
}

void AircraftManager::handleReceivedAircraft(NewAircraft& newAircraft) {
	//std::cout << newAircraft.aircraftId_ << std::endl;
	pushNewAircraftQueue(newAircraft);
}

void AircraftManager::pushNewAircraftQueue(NewAircraft& newAircraft) {
	std::lock_guard<std::mutex> lock(mtx_);
	newAircraftQueue_.push(newAircraft);
}

bool AircraftManager::popNewAircraftQueue(NewAircraft& newAircraft) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (newAircraftQueue_.empty()) {
		return false;
	}
	newAircraft = newAircraftQueue_.front();
	newAircraftQueue_.pop();
	return true;
}

bool AircraftManager::isExistAircraft(std::string& aircraftId) {
	return aircrafts_.find(aircraftId) != aircrafts_.end();
}

void AircraftManager::addAircraft(NewAircraft& newAircraft) {
	aircrafts_[newAircraft.aircraftId_] = new Aircraft(newAircraft.aircraftId_, newAircraft.location_, newAircraft.isEnemy_);
}

Aircraft* AircraftManager::getAircraft(std::string& aircraftId) {
	return aircrafts_[aircraftId];
}

void AircraftManager::judgeEngagable() {

	NewAircraftWithIP newAircraftWithIp;

	while (true) {
		if (popNewAircraftQueue(newAircraftWithIp.aircraftData_)) {

			if (!isExistAircraft(newAircraftWithIp.aircraftData_.aircraftId_)) {
				addAircraft(newAircraftWithIp.aircraftData_);
			}

			Aircraft* targetAircraft = aircrafts_[newAircraftWithIp.aircraftData_.aircraftId_];
			targetAircraft->updatePosition(newAircraftWithIp.aircraftData_.location_);

			if (!targetAircraft->isEnemy()) {	//아군 항공기
				sender_->sendAircraftData(newAircraftWithIp);
				continue;
			}

			if (targetAircraft->isIpInEngageRange(newAircraftWithIp.engagementStatus_, newAircraftWithIp.impactPoint_)) {	//적군 항공기 중 교전 가능 범위 아님
				sender_->sendAircraftData(newAircraftWithIp);
				continue;
			}

			sender_->sendAircraftData(newAircraftWithIp);
			engagementManager_->addEngagableAircraft(newAircraftWithIp.aircraftData_.aircraftId_);
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

//void AircraftManager::updateAircraftStatus() {
//
//}