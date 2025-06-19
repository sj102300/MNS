#include <iostream>
#include "EngagementManager.h"
#include "UdpReceiver.h"
#include "UdpMulticastSender.h"

EngagementManager::EngagementManager() : sender_(nullptr), multisender_(nullptr), aircraftManager_(nullptr), missileManager_(nullptr),
isRunning_(false), isChanged_(false), mode_(Mode::Auto), batteryLoc_({ -200, -200, 10 }) {

}

EngagementManager::~EngagementManager() {
	if (workThread_.joinable()) {
		workThread_.join();
	}
	sender_ = nullptr;
	multisender_ = nullptr;
	aircraftManager_ = nullptr;
	missileManager_ = nullptr;
}

bool EngagementManager::init(TCC::UdpSender* sender, AircraftManager* aircraftManager, TCC::UdpMulticastSender* multisender, MissileManager* missileManager, TCC::Position&batteryLoc) {
	if (sender == nullptr || aircraftManager == nullptr || multisender == nullptr || missileManager == nullptr)
		return false;
	sender_ = sender;
	aircraftManager_ = aircraftManager;
	multisender_ = multisender;
	missileManager_ = missileManager;
	batteryLoc_ = batteryLoc;
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

void EngagementManager::makeAutoFireCommandId(std::string& commandId) {
	using namespace std::chrono;

	// 현재 시간
	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

	// time_t → tm 변환
	std::time_t now_time_t = system_clock::to_time_t(now);
	std::tm now_tm;
	localtime_s(&now_tm, &now_time_t);  // Windows 안전 버전

	// 문자열 생성
	std::ostringstream oss;
	oss << "AF-"  // 접두사
		<< std::put_time(&now_tm, "%Y%m%d%H%M%S")
		<< std::setw(3) << std::setfill('0') << ms.count();  // 밀리초

	commandId = oss.str();
}

void EngagementManager::makeWDLCommandId(std::string& commandId) {
	using namespace std::chrono;

	// 현재 시간
	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

	// time_t → tm 변환
	std::time_t now_time_t = system_clock::to_time_t(now);
	std::tm now_tm;
	localtime_s(&now_tm, &now_time_t);  // Windows 안전 버전

	// 문자열 생성
	std::ostringstream oss;
	oss << "WF-"  // 접두사
		<< std::put_time(&now_tm, "%Y%m%d%H%M%S")
		<< std::setw(3) << std::setfill('0') << ms.count();  // 밀리초

	commandId = oss.str();
}

bool EngagementManager::mappingMissileToAircraft(std::string & aircraftId, std::string& missileId) {

	missileId = missileManager_->findAvailableMissile();
	if (missileId.empty()) {
		std::cout << "No Available Missile!" << std::endl;
		return false;		//사용가능한 미사일이 없음. 더미 데이터
	}

	
	missileToAircraft_[missileId] = aircraftId;
	return true;
}

bool EngagementManager::engagementSuccess(std::string targetAircraftId, std::string targetMissileId) {

	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	std::cout << "EngagementManager::engagementSuccess() called" << std::endl;
	std::cout << "Target Aircraft ID: " << targetAircraftId << std::endl;
	std::cout << "Target Missile ID: " << targetMissileId << std::endl;
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;

	auto it = missileToAircraft_.find(targetMissileId);
	if (it == missileToAircraft_.end()) {		//사용한 미사일이 아님
		std::cout << u8"===========================================================" << std::endl;
		std::cout << u8"사용한 미사일이 아닙니다. engagementSuccess()" << std::endl;
		std::cout << u8"===========================================================" << std::endl;
		return false;
	}

	// 미사일 상태변화는 미사일 정보 메시지 들어왔을때 업데이트.
	//missileManager_->updateMissileStatus(targetMissileId, Missile::MissileStatus::NotEngagable);

	//격추된 항공기 상태 변화
	Aircraft* targetAircraft = aircraftManager_->getAircraft(targetAircraftId);
	if (targetAircraft == nullptr) {

		std::cout << u8"탐지된 항공기가 아닙니다!" << std::endl;
		return false;
	}
	targetAircraft->updateStatus(Aircraft::EngagementStatus::Destroyed);

	//원래 쫓던 항공기 상태 변화
	std::string& originalTargetAircraftId = it->second;
	if (originalTargetAircraftId != targetAircraftId) {		//원하던 항공기를 격추한게 아님.

		std::cout << u8"===========================================================" << std::endl;
		std::cout << u8"원하던 항공기를 격추하지 못했습니다. engagementSuccess()" << std::endl;
		std::cout << u8"===========================================================" << std::endl;

		Aircraft* originalTargetAircraft = aircraftManager_->getAircraft(originalTargetAircraftId);
		if (originalTargetAircraft == nullptr) {
			std::cout << u8"교전 중인 항공기가 아닙니다!" << std::endl;
			return false;
		}
		originalTargetAircraft->updateStatus(Aircraft::EngagementStatus::NotEngageable);
		// TODO: WDL기능 추가 가능
	}

	//터진 미사일의 원래 발사 명령 찾기
	if (!handleMissileDestroyed(targetMissileId, DestroyType::EngagementSuccess)) {
		std::cout << u8"===========================================================" << std::endl;
		std::cout << u8"미사일 폭파 메시지 전송 실패! engagementSuccess()" << std::endl;
		std::cout << u8"===========================================================" << std::endl;
	}

	missileToAircraft_.erase(it);

	return true;
}

bool EngagementManager::handleMissileDestroyed(std::string& missileId, unsigned int type) {
	std::string launchCommandId = "";
	std::string aircraftId = "";
	for (auto& v : fireCommands_) {
		if (v.second.first == missileId) {
			launchCommandId = v.first;
			aircraftId = v.second.second;
			break;
		}
	}
	if (launchCommandId.empty()) {
		std::cout << u8"===========================================================" << std::endl;
		std::cout << u8"발사 명령 식별자를 찾을 수 없습니다! handleMissileDestroyed()" << std::endl;
		std::cout << u8"===========================================================" << std::endl;
		return false;
	}

	sender_->sendDestroyCommand(launchCommandId, aircraftId, missileId, type);
	return true;

}

//MissileManager에서 이거 호출할까?
//bool EngagementManager::isHitTarget(std::string& missileId) {
//	//격추 성공 수신
//	//해당 미사일이 격추하던 항공기를 올바르게 격추했는지를 판단.
//	//만약 올바르게 격추했다면 항공기의 status를 EngagementStatus::Destroyed
//	//올바르게 격추되지 않았다면 항공기의 status를 EngagementStatus::NotEngagable
//	// 
//	// 미사일-항공기 매핑 확인
//	auto it = missileToAircraft_.find(missileId);
//	if (it == missileToAircraft_.end()) {
//		// 매핑된 항공기가 없음
//		return false;
//	}
//
//	std::string& aircraftId = it->second;
//	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
//	if (!aircraft) {
//		// 항공기 객체가 존재하지 않음
//		missileToAircraft_.erase(it);
//		return false;
//	}
//
//	// 실제로 격추한 항공기가 내가 쫓던 항공기가 맞는지 확인하는 로직
//
//	//내가 쫓던 항공기의 EngagementStatus를 Engagable or NotEngagable로 변경
//	//근데 두개의 미사일이 하나를 쫓다가 하나가	격추했다면 Engagable? NotEngagable?
//	//Todo : 내가 쫓던 항공기가 아닌 항공기를 격추 한 경우  
//	// 올바르게 격추했다고 판단
//	std::cout << aircraftId << " 미사일 격추 성공 " << "\n";
//
//	// 매핑 해제
//	missileToAircraft_.erase(it);
//
//	return true;
//}

void EngagementManager::work() {

	std::string aircraftId;
	while (isRunning_) {
		std::cout << "EngagementManager work() start" << std::endl;
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

		//자동발사일때
		while (engagableAircrafts_.popQueue(aircraftId)) {
			std::string commandId;
			makeAutoFireCommandId(commandId);
			launchMissile(commandId, aircraftId);
		}
	}
}

void EngagementManager::notifyThread() {
	isChanged_ = true;
	cv_.notify_one();
	return;
}

unsigned int EngagementManager::changeMode(unsigned int mode) {
	mode_ = mode;
	notifyThread();
	return mode_;
}

bool EngagementManager::manualFire(std::string commandId, std::string targetAircraftId) {
	std::cout << "manaulFire() : " << commandId <<" "<< targetAircraftId << std::endl;
	return launchMissile(commandId, targetAircraftId);
}

bool EngagementManager::launchMissile(std::string &commandId, std::string& aircraftId) {

	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
	if(aircraft == nullptr){
		std::cout << "--------------------------------" << std::endl;
		std::cout << "No Aircraft!" << std::endl;
		std::cout << "--------------------------------" << std::endl;
		return false;
	}

	if (!aircraft->isEngagable()) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Aircraft is notEngageable" << std::endl;
		std::cout << "--------------------------------" << std::endl;
		return false;		//교전 불가능 한 항공기임.
	}

	if (!aircraft->calcImpactPoint(batteryLoc_)) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Failed to calculate impact point." << std::endl;
		std::cout << "--------------------------------" << std::endl;
		aircraft->updateStatus(Aircraft::EngagementStatus::NotEngageable);
		return false;
	};

	TCC::Position impactPoint;
	aircraft->getImpactPoint(impactPoint);
	std::cout << "lati: " << impactPoint.latitude_
		<< "longi: " << impactPoint.longitude_
		<< "alti: " << impactPoint.altitude_
		<< std::endl;

	std::string missileId;
	if (!mappingMissileToAircraft(aircraftId, missileId)) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Failed to map missile to aircraft." << std::endl;
		std::cout << "--------------------------------" << std::endl;
		return false; // 미사일 매핑 실패
	};

	unsigned int engagementStatus = aircraft->updateStatus(Aircraft::EngagementStatus::Engaging);

	//발사명령 보내기
	fireCommands_[commandId] = { missileId, aircraftId };
	multisender_->sendLaunchCommand(commandId, aircraftId, missileId, impactPoint);
	sender_->sendLaunchCommand(commandId, aircraftId, missileId, impactPoint);

	std::cout << "==================================" << std::endl;
	std::cout << "launchMissile(): " << commandId << " " << aircraftId << " " << missileId <<" success" << std::endl;
	std::cout << "impactPoint: "<< "lati: "<<impactPoint.latitude_
		<<"longi: "<<impactPoint.longitude_
		<<"alti: "<<impactPoint.altitude_ << std::endl;
	std::cout << "==================================" << std::endl;

	return true;
}

// OCC에서 비상폭파 패킷을 수신했을 때 비상폭파 명령을 멀티캐스트로 에코
// 비상폭파 완료는 Ack(미사일에서 비상폭파 상태를 보냈을때)받았을 때 판단
bool EngagementManager::emergencyDestroy(std::string commandId, std::string missileId) {
	
	multisender_->sendEmergencyDestroyCommand(commandId, missileId);
	
	bool result = multisender_->waitForAckResult(missileId, 3000); // 3초 대기
	if (result) {
		// Ack 수신 성공 처리
		missileToAircraft_.erase(missileId);
		Missile* missile = missileManager_->selectMissile(missileId);
	}
	else {
		// Ack 수신 실패 처리
	}

	//비상폭파했다치고..
	//걔가 쫓던 발사명령을 찾아와야함
	if (!handleMissileDestroyed(missileId, DestroyType::EmergencyDestroy)) {
		std::cout << u8"===========================================================" << std::endl;
		std::cout << u8"미사일 폭파 메시지 전송 실패! emergencyDestroy()" << std::endl;
		std::cout << u8"===========================================================" << std::endl;
	}

	return true;
}

void EngagementManager::addEngagableAircraft(std::string& aircraftId) {
	if (engagableAircrafts_.pushQueue(aircraftId)) {
		notifyThread();
	}
}

bool EngagementManager::EngagableAircraftQueue::pushQueue(std::string& aircraftId) {
	std::lock_guard<std::mutex> lock(mtx_);
	if (set.find(aircraftId) != set.end())
		return false;
	set.insert(aircraftId);
	queue.push(aircraftId);
	std::cout << "EngagableAircraftQueue pushQueue() : " << aircraftId << std::endl;
	return true;
}

bool EngagementManager::weaponDataLink(std::string commandId, std::string aircraftId, std::string missileId) {

	//뭘해줘야하지?
	TCC::Position impactPoint;

	Aircraft* targetAircraft = aircraftManager_->getAircraft(aircraftId);
	if (targetAircraft == nullptr) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << "No targetAircraft!" << std::endl;
		std::cout << "--------------------------------" << std::endl;
		return false;
	}

	if (!targetAircraft->isEnemy()) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << u8"아군 항공기입니다 격추할 수 없습니다!" << std::endl;
		std::cout << "--------------------------------" << std::endl;
		return false;
	}

	if (!targetAircraft->isEngagable()) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << "targetAircraft is notEngageable" << std::endl;
		std::cout << "--------------------------------" << std::endl;
		return false;		//교전 불가능 한 항공기임.
	}

	Missile* targetMissile = missileManager_->selectMissile(missileId);
	TCC::Position curMissileLoc;
	targetMissile->getCurLocation(curMissileLoc);

	if (!targetAircraft->calcImpactPoint(curMissileLoc)) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Failed to calculate impact point." << std::endl;
		std::cout << "--------------------------------" << std::endl;
		targetAircraft->updateStatus(Aircraft::EngagementStatus::NotEngageable);
		return false;
	};

	TCC::Position impactPoint;
	targetAircraft->getImpactPoint(impactPoint);
	std::cout << "lati: " << impactPoint.latitude_
		<< "longi: " << impactPoint.longitude_
		<< "alti: " << impactPoint.altitude_
		<< std::endl;

	//미사일 맵핑 수정
	missileToAircraft_[missileId] = aircraftId;

	std::string wdlCommandId;
	makeWDLCommandId(wdlCommandId);
	fireCommands_[wdlCommandId] = { missileId, aircraftId };
	//unordered_map 두개 부분 수정하기

	handleMissileDestroyed(missileId, DestroyType::WDL);		//원래있던 발사명령의 impact point 지우기 위함
	sender_->sendLaunchCommand(wdlCommandId, aircraftId, missileId, impactPoint);
	multisender_->sendWDLCommand(wdlCommandId, aircraftId, missileId, impactPoint);
}

bool EngagementManager::EngagableAircraftQueue::popQueue(std::string& aircraftId) {
	std::lock_guard<std::mutex>lock(mtx_);
	if (queue.empty())
		return false;
	//std::cout << queue.size() << " aircrafts in queue\n";
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