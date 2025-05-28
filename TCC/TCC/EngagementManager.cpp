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
	mode_ = Mode::Auto; //�ʱ� ��带 Auto�� ����
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
			continue;	//�����߻� ����϶��� �۾��Ұ� ����
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
		return;		//��밡���� �̻����� ����. ���� ������
	}

	//WDL��� �ҰŸ� �̹� �ִ� �̻������� �˻��ϱ�
	//missileToAircraft_[missileId] = aircraftId;
	return;
}

// MissileManager���� �̻����� ���°� ���� ������ �� �� �Լ��� ȣ��
// 
bool EngagementManager::isHitTarget(std::string& missileId) {
	//���� ���� ����
	//�ش� �̻����� �����ϴ� �װ��⸦ �ùٸ��� �����ߴ����� �Ǵ�.
	//���� �ùٸ��� �����ߴٸ� �װ����� status�� EngagementStatus::Destroyed
	//�ùٸ��� ���ߵ��� �ʾҴٸ� �װ����� status�� EngagementStatus::NotEngagable
	// 
	// �̻���-�װ��� ���� Ȯ��
	auto it = missileToAircraft_.find(missileId);
	if (it == missileToAircraft_.end()) {
		// ���ε� �װ��Ⱑ ����
		return false;
	}

	std::string& aircraftId = it->second;
	Aircraft* aircraft = aircraftManager_->getAircraft(aircraftId);
	if (!aircraft) {
		// �װ��� ��ü�� �������� ����
		missileToAircraft_.erase(it);
		return false;
	}

	// ������ ������ �װ��Ⱑ ���� �Ѵ� �װ��Ⱑ �´��� Ȯ���ϴ� ����
	
	//���� �Ѵ� �װ����� EngagementStatus�� Engagable or NotEngagable�� ����
	//�ٵ� �ΰ��� �̻����� �ϳ��� �Ѵٰ� �ϳ���	�����ߴٸ� Engagable? NotEngagable?
	//Todo : ���� �Ѵ� �װ��Ⱑ �ƴ� �װ��⸦ ���� �� ���  
	// �ùٸ��� �����ߴٰ� �Ǵ�
	std::cout << aircraftId << " �̻��� ���� ���� " << "\n";

	// ���� ����
	missileToAircraft_.erase(it);

	return true;
}

unsigned int EngagementManager::changeMode(unsigned int mode) {
	mode_ = mode;
	notifyThread();
	// �����߻��϶��� queueclear�ϱ�
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
		//return false;		//���� �Ұ��� �� �װ�����.
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

// OCC���� ������� ��Ŷ�� �������� �� ������� ����� ��Ƽĳ��Ʈ�� ����
// ������� �Ϸ�� Ack(�̻��Ͽ��� ������� ���¸� ��������)�޾��� �� �Ǵ�
bool EngagementManager::emergencyDestroy(std::string commandId, std::string missileId) {
	//��� ���� ��� �۽�
	Missile* missile = missileManager_->selectMissile(missileId);

	sender_->sendEmergencyDestroy(commandId, missileId);

	missileToAircraft_.erase(missileId);

	// OCC -> TCC ������� ����� ������
	// TCC -> OCC ������� ����� �޾Ҵٴ� Ack ������
	// 
	// OCC->TCC ������� ok, TCC -> MSS �̰� ���޾�, OCC�� GUI ���¸� ���� ACk�� �Ƚ��
	// TCC -> �ùķ��̼� �� ������� ����� �Ѹ���
	// 
	// TCC receiver �̻����� �����͸� ��� ���� -> status == �������
	// �� ������İ� ��� �̻����� ����������� Ȯ�� -> Ack
	//sender���� �̻��� ���¸� �����ͼ� status ������� -> true -> ���� ó�� cv
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