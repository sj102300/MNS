
#include "EngagementManager.h"

bool EngagementManager::init(TCC::UdpSender* sender) {
	if (sender == nullptr)
		return false;
	sender_ = sender;
	return true;
}

void EngagementManager::start() {
	workThread_ = std::thread(&EngagementManager::work, this);
}

void EngagementManager::work() {

	while (true) {
		std::unique_lock<std::mutex> lock(mtx_);

		// mode가 0이고 작업이 있을 때만 깨어남
		cv_.wait(lock, [this] {
			return mode_ == Mode::Auto && !engagableAircrafts_.empty();
		});

		// queue에서 작업 하나 꺼냄
		std::string aircraftId;
		engagableAircrafts_.popQueue(aircraftId);
		lock.unlock();

		// 실제 작업 처리
		launchMissile(aircraftId);
	}
}

bool EngagementManager::mappingMissileToAircraft(std::string& aircraftId) {

	//std::string missileId = missileManager->selectMissile();
	
	//if (missileId.empty()) {
	//	return false;		//사용가능한 미사일이 없음
	//}

	//WDL기능 할거면 이미 있는 미사일인지 검사하기
	//missileToAircraft_[missileId] = aircraftId;
	return true;
}

bool EngagementManager::isHitTarget() {
	//격추 성공 수신
	//해당 미사일이 격추하던 항공기를 올바르게 격추했는지를 판단.
	//만약 올바르게 격추했다면 항공기의 status를 EngagementStatus::Destroyed
	//올바르게 격추되지 않았다면 항공기의 status를 EngagementStatus::NotEngagable
	return true;
}

unsigned int EngagementManager::changeMode(unsigned int mode) {
	mode_ = mode;
	if (mode_ == Mode::Auto) {
		cv_.notify_one();
	}
	else {
		engagableAircrafts_.clear();
	}
	return mode_;
}

bool EngagementManager::manualFire(std::string commandId, std::string targetAircraftId) {
	std::cout << "manaulFire() : " << commandId << std::endl;
	return launchMissile(targetAircraftId);
}

bool EngagementManager::launchMissile(std::string& aircraftId) {

	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
	if (!aircraft->isEngagable()) {
		return false;		//교전 불가능 한 항공기임.
	}

	mappingMissileToAircraft(aircraftId);
	unsigned int engagementStatus = aircraft->updateStatus(Aircraft::EngagementStatus::Engaging);
	//sender_->sendLaunchCommand();
	return true;
}

bool EngagementManager::emergencyDestroy(std::string commandId, std::string missileId) {
	//비상폭파할 미사일에 대해서
	//비상 폭파 명령 송신
	
	//Missile* missile = missileManager_->getMissile(missileId);

	//sender_->sendEmergencyDestroy(missileId);

	missileToAircraft_.erase(missileId);

	//return false?true/
	return true;
}

void EngagementManager::addEngagableAircraft(std::string& aircraftId) {
	if (mode_ == Mode::Manual)
		return;

	if (engagableAircrafts_.pushQueue(aircraftId))
		cv_.notify_one();
}

bool EngagementManager::EngagableAircraftQueue::pushQueue(std::string& aircraftId) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (set.find(aircraftId) != set.end())
		return false;
	set.insert(aircraftId);
	queue.push(aircraftId);
	return true;
}

bool EngagementManager::EngagableAircraftQueue::popQueue(std::string& aircraftId) {
	std::lock_guard<std::mutex>lock(mtx_);
	if (queue.empty())
		return false;
	aircraftId = queue.front();
	queue.pop();
	set.erase(aircraftId);
	return true;
}

bool EngagementManager::EngagableAircraftQueue::empty() {
	std::lock_guard<std::mutex>lock(mtx_);
	return queue.empty();
}

void EngagementManager::EngagableAircraftQueue::clear() {
	std::lock_guard<std::mutex>lock(mtx_);
	set.clear();
	while (!queue.empty()) {
		queue.pop();
	}
}