
#include "SimulationManager.h"

SimulationManager::SimulationManager():isRunning_(false), isChanged_(false) {
	scenarioManager_ = new sm::ScenarioManager(
		"http://192.168.2.77:8080",     // TCC Http ì„œë²„ ì£¼ì†Œ
		"http://192.168.2.30:8080",    // SCN Http ì„œë²„ ì£¼ì†Œ
		"TCC"         // í´ë¼ì´ì–¸íŠ¸ ID
	);

	scenarioManager_->setOnReadyCallback([this]() {
		std::lock_guard<std::mutex> lock(mtx_);

		if (isRunning_) {       //?´ë? ?œë‚˜ë¦¬ì˜¤ê°€ ?¤í–‰ ì¤?
			return;
		}

		isRunning_ = true;
		isChanged_ = true;
		cv_.notify_one();

		});

	scenarioManager_->setOnQuitCallback([this]() {
		std::lock_guard<std::mutex> lock(mtx_);

		if (!isRunning_) {		//?¤í–‰ì¤‘ì¸ ?œë‚˜ë¦¬ì˜¤ê°€ ?†ìŒ
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

	//httpì„œë²„ ì‹œìž‘

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

	sm::Coordinate battery = scenarioManager_->getBatteryLocation();

	TCC::Position batteryLocation = {
		battery.latitude,
		battery.longitude,
		battery.altitude
	};

	if (!createObjects(batteryLocation)) {
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
	delete multiSender_;

	return true;
}

bool SimulationManager::createObjects(TCC::Position& batteryLocation) {

	multiReceiver_ = new TCC::UdpMulticastReceiver("239.0.0.1", 9000);		//192.168.2.190ìœ¼ë¡œ ìˆ˜ì‹ 
	udpSender_ = new TCC::UdpSender("192.168.2.200", 9000);					//OCC êµì „ë§
	multiSender_ = new TCC::UdpMulticastSender("239.0.0.1", 9000);			//192.168.2.194ë¡œ ì†¡ì‹ 
	aircraftManager_ = new AircraftManager(batteryLocation);
	engagementManager_ = new EngagementManager();
	missileManager_ = new MissileManager(udpSender_, engagementManager_);
	udpReceiver_ = new TCC::UdpReceiver("192.168.2.189", 9999);				//TCC êµì „ë§

	if (!aircraftManager_->init(udpSender_, engagementManager_)) {
		std::cout << "aircraftManager init() Failed\n";
		return false;
	}
	if (!multiReceiver_->init(aircraftManager_, missileManager_, engagementManager_)) {
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

	// httpì„œë²„ ì¢…ë£Œ ì²˜ë¦¬
	//if (sm::scenarioThread.joinable()) {
	//	scenarioThread.join();
	//}

	delete scenarioManager_;
}
