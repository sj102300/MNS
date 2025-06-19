
#include "AircraftManager.h"
#include "UdpSender.h"
#include "EngagementManager.h"

AircraftManager::AircraftManager() {
	batteryLoc_ = { 0, 0, 0 }; // 초기 배터리 위치 설정
	isRunning_ = false;
	engagementManager_ = nullptr;
	sender_ = nullptr;
	workThread_ = std::thread();
}

AircraftManager::~AircraftManager() {
	for (auto& pair : aircrafts_) {
		delete pair.second;  // 각 Aircraft* 삭제
	}
	aircrafts_.clear();       // map 자체는 자동 소멸되지만 명시적으로 비워줘도 OK
}

bool AircraftManager::init(TCC::UdpSender* sender, EngagementManager* engagementManager, TCC::Position &batteryLoc) {
	if (sender == nullptr || engagementManager == nullptr) {
		return false;
	}
	engagementManager_ = engagementManager;
	sender_ = sender;
	batteryLoc_ = batteryLoc;
	return true;
}

void AircraftManager::start() {
	isRunning_ = true;
	workThread_ = std::thread(&AircraftManager::judgeEngagable, this);
}

void AircraftManager::handleReceivedAircraft(NewAircraft& newAircraft) {
	//std::cout << std::fixed << std::setprecision(9); // 소수점 9자리까지 고정 출력
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

	std::cout << "AircraftManager judgeEngagable() called" << std::endl;

	NewAircraftWithIP newAircraftWithIp;

	while (isRunning_) {
		if (popNewAircraftQueue(newAircraftWithIp.aircraftData_)) {

			if (!isExistAircraft(newAircraftWithIp.aircraftData_.aircraftId_)) {
				addAircraft(newAircraftWithIp.aircraftData_); 
			}

			Aircraft* targetAircraft = aircrafts_[newAircraftWithIp.aircraftData_.aircraftId_];
			targetAircraft->updatePosition(newAircraftWithIp.aircraftData_.location_);

			if (targetAircraft->isEnemy()) {
				if (targetAircraft->hasBecomeEngageable(batteryLoc_, newAircraftWithIp.engagementStatus_)) {
					engagementManager_->addEngagableAircraft(newAircraftWithIp.aircraftData_.aircraftId_);
				}
			}
			sender_->sendAircraftData(newAircraftWithIp);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
