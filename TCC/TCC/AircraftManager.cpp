
#include "AircraftManager.h"
#include "UdpSender.h"
#include "EngagementManager.h"

AircraftManager::AircraftManager(TCC::Position& batteryLocation) : batteryLocation_(batteryLocation) {

}

AircraftManager::~AircraftManager() {
	for (auto& pair : aircrafts_) {
		delete pair.second;  // 각 Aircraft* 삭제
	}
	aircrafts_.clear();       // map 자체는 자동 소멸되지만 명시적으로 비워줘도 OK
}

bool AircraftManager::init(TCC::UdpSender* sender, EngagementManager* engagementManager) {
	if (sender == nullptr || engagementManager == nullptr) {
		return false;
	}
	engagementManager_ = engagementManager;
	sender_ = sender;
	return true;
}

void AircraftManager::start() {
	isRunning_ = true;
	workThread_ = std::thread(&AircraftManager::judgeEngagable, this);
}

void AircraftManager::handleReceivedAircraft(NewAircraft& newAircraft) {
	std::cout << std::fixed << std::setprecision(9); // 소수점 9자리까지 고정 출력

	std::cout << "Aircraft ID: " << newAircraft.aircraftId_
		<< ", Latitude: " << newAircraft.location_.latitude_
		<< ", Longitude: " << newAircraft.location_.longitude_ << std::endl;
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

	auto it = aircrafts_.find(aircraftId);
	if (it != aircrafts_.end()) {
		return it->second;
	}
	return nullptr;
}

void AircraftManager::judgeEngagable() {

	NewAircraftWithIP newAircraftWithIp;

	while (isRunning_) {
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
			bool isEngagementStatusChanged = false;
			if (targetAircraft->isIpInEngageRange(batteryLocation_, newAircraftWithIp.engagementStatus_, newAircraftWithIp.impactPoint_, isEngagementStatusChanged)) {	//적군 항공기 중 교전 가능 범위 아님
				sender_->sendAircraftData(newAircraftWithIp);
			}

			//교전 불가능에서 교전 가능 상태로 바뀌었을때만 addEngagableAircraft호출해야함
			if (isEngagementStatusChanged) {
				engagementManager_->addEngagableAircraft(newAircraftWithIp.aircraftData_.aircraftId_);
			}

			if (newAircraftWithIp.engagementStatus_ == Aircraft::EngagementStatus::Engageable) {
				std::cout << "findEngagableAircraft Data: " << newAircraftWithIp.aircraftData_.aircraftId_ << std::endl;
			}

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void AircraftManager::stop() {
	isRunning_ = false;
	//스레드 종료
	if (workThread_.joinable()) {
		workThread_.join();
	}
	return;
}
