#include <iostream>
#include "EngagementManager.h"
#include "UdpReceiver.h"
#include "UdpMulticastSender.h"

EngagementManager::EngagementManager() : sender_(nullptr), multisender_(nullptr), aircraftManager_(nullptr), missileManager_(nullptr),
isRunning_(false), isChanged_(false), mode_(Mode::Auto) {

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

void EngagementManager::makeAutoFireCommandId(std::string &commandId) {
	using namespace std::chrono;

	// 현재 시간
	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

	// time_t → tm 변환 (스레드 안전하게)
	std::time_t now_time_t = system_clock::to_time_t(now);
	std::tm now_tm;
	localtime_s(&now_tm, &now_time_t);  // Windows 안전한 버전

	// 문자열 생성
	std::ostringstream oss;
	oss << std::put_time(&now_tm, "%Y%m%d%H%M%S") << std::setw(3) << std::setfill('0') << ms.count();

	commandId = oss.str();

	return;
}


bool EngagementManager::mappingMissileToAircraft(std::string & aircraftId, std::string& missileId) {

	missileId = missileManager_->findAvailableMissile();
	
	if (missileId.empty()) {
		std::cout << "No Available Missile!" << std::endl;
		return false;		//사용가능한 미사일이 없음. 더미 데이터
	}

	//WDL기능 할거면 이미 있는 미사일인지 검사하기
	missileToAircraft_[missileId] = aircraftId;
	return true;
}

// MissileManager에서 미사일의 상태가 격추 성공일 때 이 함수를 호출
// 
bool EngagementManager::isHitTarget(std::string& missileId) {
	//격추 성공 수신
	//해당 미사일이 격추하던 항공기를 올바르게 격추했는지를 판단.
	//만약 올바르게 격추했다면 항공기의 status를 EngagementStatus::Destroyed
	//올바르게 격추되지 않았다면 항공기의 status를 EngagementStatus::NotEngagable
	
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
		if (engagableAircrafts_.popQueue(aircraftId)) {
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
	// 수동발사일때는 queueclear하기
	// engagableAircrafts_.clear();
	return mode_;
}

bool EngagementManager::manualFire(std::string commandId, std::string targetAircraftId) {
	std::cout << "manaulFire() : " << commandId <<" "<< targetAircraftId << std::endl;
	return launchMissile(commandId, targetAircraftId);
}

bool EngagementManager::launchMissile(std::string &commandId, std::string& aircraftId) {

	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
	if(aircraft == nullptr){
		std::cout << "No Aircraft!" << std::endl;
		return false;
	}

	TCC::Position impactPoint;
	
	if (!aircraft->isEngagable()) {
		std::cout << "aircraft is notEngagable" << std::endl;
		aircraft->getImpactPoint(impactPoint);
		std::cout << "lati: " << impactPoint.latitude_
			<< "longi: " << impactPoint.longitude_
			<< "alti: " << impactPoint.altitude_
			<< std::endl;
		return false;		//교전 불가능 한 항공기임.
	}

	std::string missileId;
	if (!mappingMissileToAircraft(aircraftId, missileId)) {
		std::cout << "Failed to map missile to aircraft." << std::endl;
		return false; // 미사일 매핑 실패
	};

	unsigned int engagementStatus = aircraft->updateStatus(Aircraft::EngagementStatus::Engaging);
	aircraft->getImpactPoint(impactPoint);

	multisender_->sendLaunchCommand(commandId, aircraftId, missileId, impactPoint);

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
	//missileToAircraft_.erase(missileId);
	//Missile* missile = missileManager_->selectMissile(missileId);
	
	// TCC receiver 미사일의 데이터를 계속 수신 -> status == 비상폭파
	// 이 비상폭파가 어느 미사일의 비상폭파인지 확인 -> Ack
	//sender에서 미사일 상태를 가져와서 status 비상폭파 -> true -> 동기 처럼 cv
	//return false/true

	return true;
}

void EngagementManager::addEngagableAircraft(std::string& aircraftId) {
	if (mode_ == Mode::Manual)
		return;
	
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