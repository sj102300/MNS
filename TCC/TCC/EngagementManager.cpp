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
	mode_ = Mode::Auto; //�ʱ� ��带 Auto�� ����
	workThread_ = std::thread(&EngagementManager::work, this);
}

void EngagementManager::stop() {
	isRunning_ = false;
	isChanged_ = true;
	cv_.notify_one();
}

void EngagementManager::makeAutoFireCommandId(std::string &commandId) {
	using namespace std::chrono;

	// ���� �ð�
	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

	// time_t �� tm ��ȯ (������ �����ϰ�)
	std::time_t now_time_t = system_clock::to_time_t(now);
	std::tm now_tm;
	localtime_s(&now_tm, &now_time_t);  // Windows ������ ����

	// ���ڿ� ����
	std::ostringstream oss;
	oss << std::put_time(&now_tm, "%Y%m%d%H%M%S") << std::setw(3) << std::setfill('0') << ms.count();

	commandId = oss.str();

	return;
}

bool EngagementManager::mappingMissileToAircraft(std::string & aircraftId, std::string& missileId) {

	missileId = missileManager_->findAvailableMissile();
	
	if (missileId.empty()) {
		std::cout << "No Available Missile!" << std::endl;
		return false;		//��밡���� �̻����� ����. ���� ������
	}

	//WDL��� �ҰŸ� �̹� �ִ� �̻������� �˻��ϱ�
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
	if (it == missileToAircraft_.end()) {		//����� �̻����� �ƴ�
		return false;
	}

	// �̻��� ���º�ȭ�� �̻��� ���� �޽��� �������� ������Ʈ.
	//missileManager_->updateMissileStatus(targetMissileId, Missile::MissileStatus::NotEngagable);

	//���ߵ� �װ��� ���� ��ȭ
	Aircraft* targetAircraft = aircraftManager_->getAircraft(targetAircraftId);
	if (targetAircraft == nullptr) {
		std::cout << "Ž���� �װ��Ⱑ �ƴմϴ�!" << std::endl;
		return false;
	}
	targetAircraft->updateStatus(Aircraft::EngagementStatus::Destroyed);

	//���� �Ѵ� �װ��� ���� ��ȭ
	std::string& originalTargetAircraftId = it->second;
	if (originalTargetAircraftId != targetAircraftId) {		//���ϴ� �װ��⸦ �����Ѱ� �ƴ�.
		Aircraft* originalTargetAircraft = aircraftManager_->getAircraft(originalTargetAircraftId);
		if (originalTargetAircraft == nullptr) {
			std::cout << "���� ���� �װ��Ⱑ �ƴմϴ�!" << std::endl;
			return false;
		}
		originalTargetAircraft->updateStatus(Aircraft::EngagementStatus::NotEngageable);
		// TODO: WDL��� �߰� ����
	}

	missileToAircraft_.erase(it);

	return true;
}

//MissileManager���� �̰� ȣ���ұ�?
//bool EngagementManager::isHitTarget(std::string& missileId) {
//	//���� ���� ����
//	//�ش� �̻����� �����ϴ� �װ��⸦ �ùٸ��� �����ߴ����� �Ǵ�.
//	//���� �ùٸ��� �����ߴٸ� �װ����� status�� EngagementStatus::Destroyed
//	//�ùٸ��� ���ߵ��� �ʾҴٸ� �װ����� status�� EngagementStatus::NotEngagable
//	// 
//	// �̻���-�װ��� ���� Ȯ��
//	auto it = missileToAircraft_.find(missileId);
//	if (it == missileToAircraft_.end()) {
//		// ���ε� �װ��Ⱑ ����
//		return false;
//	}
//
//	std::string& aircraftId = it->second;
//	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
//	if (!aircraft) {
//		// �װ��� ��ü�� �������� ����
//		missileToAircraft_.erase(it);
//		return false;
//	}
//
//	// ������ ������ �װ��Ⱑ ���� �Ѵ� �װ��Ⱑ �´��� Ȯ���ϴ� ����
//
//	//���� �Ѵ� �װ����� EngagementStatus�� Engagable or NotEngagable�� ����
//	//�ٵ� �ΰ��� �̻����� �ϳ��� �Ѵٰ� �ϳ���	�����ߴٸ� Engagable? NotEngagable?
//	//Todo : ���� �Ѵ� �װ��Ⱑ �ƴ� �װ��⸦ ���� �� ���  
//	// �ùٸ��� �����ߴٰ� �Ǵ�
//	std::cout << aircraftId << " �̻��� ���� ���� " << "\n";
//
//	// ���� ����
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
			continue;	//�����߻� ����϶��� �۾��Ұ� ����
		}

		//�ڵ��߻��϶�
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

	TCC::Position impactPoint;
	if (!aircraft->isEngagable()) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Aircraft is notEngageable" << std::endl;
		aircraft->getImpactPoint(impactPoint);
		std::cout << "lati: " << impactPoint.latitude_
			<< "longi: " << impactPoint.longitude_
			<< "alti: " << impactPoint.altitude_
			<< std::endl;
		std::cout << "--------------------------------" << std::endl;
		return false;		//���� �Ұ��� �� �װ�����.
	}

	std::string missileId;
	if (!mappingMissileToAircraft(aircraftId, missileId)) {
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Failed to map missile to aircraft." << std::endl;
		std::cout << "--------------------------------" << std::endl;
		return false; // �̻��� ���� ����
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

// OCC���� ������� ��Ŷ�� �������� �� ������� ����� ��Ƽĳ��Ʈ�� ����
// ������� �Ϸ�� Ack(�̻��Ͽ��� ������� ���¸� ��������)�޾��� �� �Ǵ�
bool EngagementManager::emergencyDestroy(std::string commandId, std::string missileId) {
	
	multisender_->sendEmergencyDestroyCommand(commandId, missileId);

	bool result = multisender_->waitForAckResult(missileId, 3000); // 3�� ���
	if (result) {
		// Ack ���� ���� ó��
		missileToAircraft_.erase(missileId);
		Missile* missile = missileManager_->selectMissile(missileId);
	}
	else {
		// Ack ���� ���� ó��
	}
	
	// TCC receiver �̻����� �����͸� ��� ���� -> status == �������
	// �� ������İ� ��� �̻����� ����������� Ȯ�� -> Ack
	//sender���� �̻��� ���¸� �����ͼ� status ������� -> true -> ���� ó�� cv
	//return false/true

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