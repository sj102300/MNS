#pragma once

//#include "CommandReceiver.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <queue>
#include <thread>
#include <atomic>
#include "UdpSender.h"

class MissileManager;
class AircraftManager;

class EngagementManager {
public:

	void start();
	bool init(TCC::UdpSender* sender);
	bool isHitTarget();
	unsigned int changeMode(unsigned int mode);
	bool launchMissile(std::string& aircraftId);
	bool emergencyDestroy(std::string commandId, std::string missileId);
	void addEngagableAircraft(std::string& aircraftId);
	bool manualFire(std::string commandId, std::string targetAircraftId);

private:

	enum Mode {
		Auto = 0,
		Manual = 1,
	};

	class EngagableAircraftQueue {
	public:
		std::mutex mtx_;
		std::queue<std::string> queue;
		std::unordered_set<std::string> set;

		bool pushQueue(std::string& aircraftId);
		bool popQueue(std::string& aircraftId);
		bool empty();
		void clear();
	};

	std::mutex mtx_;
	std::condition_variable cv_;

	TCC::UdpSender* sender_;
	//TCC::UdpMulticastSender* multisender_;
	MissileManager* missileManager_;
	AircraftManager* aircraftManager_;
	std::unordered_map<std::string, std::string> missileToAircraft_;
	std::thread workThread_;
	std::atomic<unsigned int> mode_;
	EngagableAircraftQueue engagableAircrafts_;

	void work();
	bool mappingMissileToAircraft(std::string& aircraftId);
};


//
//class EngagableAircraftQueue {
//public:
//	typedef struct _engagable_aircraft {
//		std::string aircraftId_;
//		Aircraft* aircraft_;
//	} EngagableAircraft;
//
//	std::mutex mtx_;
//	std::queue<EngagableAircraft> queue;
//	std::unordered_set<std::string> set;
//
//	bool pushQueue(std::string& aircraftId, Aircraft* aircraft);
//	bool popQueue(EngagableAircraft& engagableAircraft);
//	bool empty();
//	void clear();
//};