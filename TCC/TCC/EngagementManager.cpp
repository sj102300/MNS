#include <iostream>
#include "EngagementManager.h"
#include "UdpReceiver.h"
#include "UdpMulticastSender.h"

bool EngagementManager::init(TCC::UdpSender* sender, AircraftManager* aircraftManager, TCC::UdpMulticastSender* multisender, MissileManager* missileManager) {
	if (sender == nullptr || aircraftManager == nullptr || multisender == nullptr || missileManager == nullptr)
		return false;
	sender_ = sender;
	aircraftManager_ = aircraftManager;
	multisender_ = multisender;
	missileManager_ = missileManager;
	return true;
}

void EngagementManager::start() {
	isRunning_ = true;
	isChanged_ = false;
	mode_ = Mode::Auto; //초기 모드를 Auto로 설정
	workThread_ = std::thread(&EngagementManager::work, this);
}

void EngagementManager::stop() {
	isRunning_ = false;
	isChanged_ = true;
	cv_.notify_one();
}

void EngagementManager::work() {

	std::string aircraftId;
	while (isRunning_) {
		{
			std::unique_lock<std::mutex> lock(mtx_);
			cv_.wait(lock, [this] { return isChanged_.load(); });
			isChanged_ = false;
		}

		if (!isRunning_)
			break;

		if (mode_ == Mode::Manual) {
			continue;	//수동발사 모드일때는 작업할게 없음
		}

		if(engagableAircrafts_.popQueue(aircraftId)){
			std::string commandId = "AF-20250528000000000";
			launchMissile(commandId, aircraftId);
		}
	}
}

void EngagementManager::mappingMissileToAircraft(std::string & aircraftId, std::string& missileId) {

	std::cout << "mappingMissileToAircraft() called\n" << std::endl;
	missileId = missileManager_->findAvailableMissile();
	
	if (missileId.empty()) {
		missileId = "MSS-100";
		return;		//사용가능한 미사일이 없음. 더미 데이터
	}

	//WDL기능 할거면 이미 있는 미사일인지 검사하기
	//missileToAircraft_[missileId] = aircraftId;
	return;
}

// MissileManager에서 미사일의 상태가 격추 성공일 때 이 함수를 호출
// 
bool EngagementManager::isHitTarget(std::string& missileId) {
	//격추 성공 수신
	//해당 미사일이 격추하던 항공기를 올바르게 격추했는지를 판단.
	//만약 올바르게 격추했다면 항공기의 status를 EngagementStatus::Destroyed
	//올바르게 격추되지 않았다면 항공기의 status를 EngagementStatus::NotEngagable
	// 
	// 미사일-항공기 매핑 확인
	auto it = missileToAircraft_.find(missileId);
	if (it == missileToAircraft_.end()) {
		// 매핑된 항공기가 없음
		return false;
	}

	std::string& aircraftId = it->second;
	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
	if (!aircraft) {
		// 항공기 객체가 존재하지 않음
		missileToAircraft_.erase(it);
		return false;
	}

	// 실제로 격추한 항공기가 내가 쫓던 항공기가 맞는지 확인하는 로직
	
	//내가 쫓던 항공기의 EngagementStatus를 Engagable or NotEngagable로 변경
	//근데 두개의 미사일이 하나를 쫓다가 하나가	격추했다면 Engagable? NotEngagable?
	//Todo : 내가 쫓던 항공기가 아닌 항공기를 격추 한 경우  
	// 올바르게 격추했다고 판단
	std::cout << aircraftId << " 미사일 격추 성공 " << "\n";

	// 매핑 해제
	missileToAircraft_.erase(it);

	return true;
}

unsigned int EngagementManager::changeMode(unsigned int mode) {
	mode_ = mode;
	notifyThread();
	// 수동발사일때는 queueclear하기
		//engagableAircrafts_.clear();
	return mode_;
}

bool EngagementManager::manualFire(std::string commandId, std::string targetAircraftId) {
	std::cout << "manaulFire() : " << commandId << std::endl;
	return launchMissile(commandId, targetAircraftId);
}

bool EngagementManager::launchMissile(std::string &commandId, std::string& aircraftId) {

	std::cout << "launchMissile(): " << commandId << " " << aircraftId << " " << std::endl;
	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);

	if(aircraft == nullptr){
		std::cout << "aircraft is nullptr!" << std::endl;
		return false;
	}

	//if (aircraft->isEnemy()) {
	//	std::cout << "aircraft is Enemy" << std::endl;
	//	return false;
	//}

	//if (!aircraft->isEngagable()) {
		//std::cout << "aircraft is notEngagable" << std::endl;
		//TCC::Position impactpoint = aircraft->getImpactPoint();
		//std::cout << "lati: " << impactpoint.latitude_
		//	<< "longi: " << impactpoint.longitude_
		//	<< "alti: " << impactpoint.altitude_
		//	<< std::endl;
		//return false;		//교전 불가능 한 항공기임.
	//}

	std::string missileId;
	mappingMissileToAircraft(aircraftId, missileId);

	unsigned int engagementStatus = aircraft->updateStatus(Aircraft::EngagementStatus::Engaging);
	TCC::Position impactPoint = aircraft->getImpactPoint();

	//std::string missileId = "MSS-001";
	//std::string aircraftID = "ATS-0001";
	//TCC::Position impactPoint = { 37.011, 128.054, 10 };
	multisender_->sendLaunchCommand(commandId, aircraftId, missileId, impactPoint);

	//std::cout << "launchMissile() success" << std::endl;
	return true;
}

// OCC에서 비상폭파 패킷을 수신했을 때 비상폭파 명령을 멀티캐스트로 에코
// 비상폭파 완료는 Ack(미사일에서 비상폭파 상태를 보냈을때)받았을 때 판단
bool EngagementManager::emergencyDestroy(std::string commandId, std::string missileId) {
	//비상 폭파 명령 송신
	Missile* missile = missileManager_->selectMissile(missileId);

	sender_->sendEmergencyDestroy(commandId, missileId);

	missileToAircraft_.erase(missileId);

	// OCC -> TCC 비상폭파 명령을 내린다
	// TCC -> OCC 비상폭파 명령을 받았다는 Ack 보내기
	// 
	// OCC->TCC 비상폭파 ok, TCC -> MSS 이걸 못받아, OCC의 GUI 상태를 변경 ACk를 안쏘고
	// TCC -> 시뮬레이션 망 비상폭파 명령을 뿌린다
	// 
	// TCC receiver 미사일의 데이터를 계속 수신 -> status == 비상폭파
	// 이 비상폭파가 어느 미사일의 비상폭파인지 확인 -> Ack
	//sender에서 미사일 상태를 가져와서 status 비상폭파 -> true -> 동기 처럼 cv
	//return false?true/

	return true;
}

void EngagementManager::addEngagableAircraft(std::string& aircraftId) {

	if (mode_ == Mode::Manual)
		return;

	if (engagableAircrafts_.pushQueue(aircraftId)) {
		notifyThread();
	}
}

void EngagementManager::notifyThread() {
	isChanged_ = true;
	cv_.notify_one();
	return;
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