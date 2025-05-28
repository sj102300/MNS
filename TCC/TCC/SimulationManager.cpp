
#include "SimulationManager.h"

SimulationManager::SimulationManager() {
	scenarioManager_ = new sm::ScenarioManager(
		"http://192.168.2.66:8080",     // 수신 주소
		"http://192.168.2.30:8080",    // SCN 서버 주소
		"TCC"         // 클라이언트 ID
	);

	scenarioManager_->setOnReadyCallback([this]() {
		std::lock_guard<std::mutex> lock(mtx_);

		if (isRunning_) {       //이미 시나리오가 실행 중
			return;
		}

		isRunning_ = true;
		isChanged_ = true;
		cv_.notify_one();

		});

	scenarioManager_->setOnQuitCallback([this]() {
		std::lock_guard<std::mutex> lock(mtx_);

		if (!isRunning_) {		//실행중인 시나리오가 없음
			return;
		}

		isRunning_ = false;
		isChanged_ = true;
		cv_.notify_one();

		});
}

void SimulationManager::startSimulation() {

	//http서버 시작
	std::thread scenarioThread([&]() {
		scenarioManager_->run();
		});

	while (true) {
		{
			std::unique_lock<std::mutex> lock(mtx_);
			cv_.wait(lock, [this] { return isChanged_.load(); });
			isChanged_ = false;
		}

		if (isRunning_) {
			startScenario();
		}
		else {
			quitScenario();
		}
	}
}

bool SimulationManager::startScenario() {

	if (!createObjects()) {
		std::cout << "startScenario() Failed\n";
		return false;
	}

	std::cout << "startScenario()" << std::endl;

	multiReceiver_->start();
	aircraftManager_->start();

	return true;
}

bool SimulationManager::quitScenario() {

	std::cout << "quitScenario()" << std::endl;
	delete aircraftManager_;
	delete engagementManager_;
	delete udpSender_;
	delete multiReceiver_;

	return true;
}

bool SimulationManager::createObjects() {
	multiReceiver_ = new TCC::UdpMulticastReceiver("239.0.0.1", 9000);
	udpSender_ = new TCC::UdpSender("192.168.2.200", 9000); //OCC  ּ 
	aircraftManager_ = new AircraftManager();
	engagementManager_ = new EngagementManager();
	missileManager_ = new MissileManager(udpSender_, engagementManager_);

	if (!aircraftManager_->init(udpSender_, engagementManager_)) {
		std::cout << "aircraftManager init() Failed\n";
		return false;
	}
	if (!multiReceiver_->init(aircraftManager_, missileManager_)) {
		std::cout << "multiReceiver init() Failed\n";
		return false;
	}
	if (!udpSender_->init()) {
		std::cout << "udpSender init() Failed\n";
		return false;
	}
	if (!engagementManager_->init(udpSender_)) {
		std::cout << "engagement manager init() Failed\n";
		return false;
	}

	return true;
}


void SimulationManager::quitSimulation() {

	std::cout << "quitSimulation()" << std::endl;
	//    // http서버 종료 처리
	//    if (scenarioThread.joinable()) scenarioThread.join();
	//    if (radarThread.joinable()) radarThread.join();
	//
	//    std::cout << u8"[" << SUBSYSTEM_ID << u8"] 프로그램 정상 종료\n";
	//    return 0;

	delete scenarioManager_;
}
