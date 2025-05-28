
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

	//startScenario();
	//while (1);

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

	//sm::Coordinate batterylocation = scenarioManager_->getBatteryLocation();

	if (!createObjects()) {
		std::cout << "startScenario() Failed\n";
		return false;
	}
	std::cout << "startScenario()" << std::endl;

	multiReceiver_->start();
	std::cout << "multiReceiver start()" << std::endl;
	aircraftManager_->start();
	std::cout << "aircraftManager_ start()" << std::endl;
	udpReceiver_->start();
	std::cout << "udpReceiver_ start()" << std::endl;
	engagementManager_->start();
	std::cout << "engagementManager_ start()" << std::endl;

	return true;
}

bool SimulationManager::quitScenario() {

	std::cout << "quitScenario()" << std::endl;

	aircraftManager_->stop();
	std::cout << "aircraftManager stop()" << std::endl;
	engagementManager_->stop();
	std::cout << "engagementManager stop()" << std::endl;
	multiReceiver_->stop();
	std::cout << "multiReceiver stop()" << std::endl;
	udpReceiver_->stop();
	std::cout << "udpReceiver stop()" << std::endl;

	delete aircraftManager_;
	delete engagementManager_;
	delete missileManager_;
	delete udpSender_;
	delete udpReceiver_;
	delete multiReceiver_;

	return true;
}

bool SimulationManager::createObjects() {
	multiReceiver_ = new TCC::UdpMulticastReceiver("239.0.0.1", 9000);
	udpSender_ = new TCC::UdpSender("192.168.2.200", 9000); //OCC  ּ 
	multiSender_ = new TCC::UdpMulticastSender("239.0.0.1", 9000);
	aircraftManager_ = new AircraftManager();
	engagementManager_ = new EngagementManager();
	missileManager_ = new MissileManager(udpSender_, engagementManager_);
	udpReceiver_ = new TCC::UdpReceiver("192.168.2.100", 9000);
	

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
	if (!engagementManager_->init(udpSender_, aircraftManager_, multiSender_, missileManager_)) {
		std::cout << "engagement manager init() Failed\n";
		return false;
	}
	if (!udpReceiver_->init(engagementManager_)) {
		std::cout << "udpReceiver init() Failed\n";
		return false;
	}
	if (!multiSender_->init()) {
		std::cout << "multiSender init() Failed\n";
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
