#pragma once

//#include "CommandReceiver.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <queue>
#include <thread>
#include <atomic>
#include "UdpSender.h"
#include "UdpMulticastSender.h"

class MissileManager;
class AircraftManager;

class EngagementManager {
public:
	void start();
	void stop();
	bool init(TCC::UdpSender* sender, AircraftManager* aircraftManager, TCC::UdpMulticastSender* multisender, MissileManager* missileManager);
	bool isHitTarget(std::string& missileId);
	unsigned int changeMode(unsigned int mode);
	bool launchMissile(std::string& commandId, std::string& aircraftId);
	bool emergencyDestroy(std::string commandId, std::string missileId);
	void addEngagableAircraft(std::string& aircraftId);
	bool manualFire(std::string commandId, std::string targetAircraftId);
	void notifyThread();

private:
	enum Mode {
		Auto = 0,
		Manual = 1,
	};

	class EngagableAircraftQueue {
	public:
		std::mutex mtx_;		//queue��
		std::queue<std::string> queue;
		std::unordered_set<std::string> set;

		bool pushQueue(std::string& aircraftId);
		bool popQueue(std::string& aircraftId);
		bool empty();
		void clear();
	};

	std::mutex mtx_;				//cv_��
	std::condition_variable cv_;

	TCC::UdpSender* sender_;
	TCC::UdpMulticastSender * multisender_;
	MissileManager* missileManager_;
	AircraftManager* aircraftManager_;
	std::unordered_map<std::string, std::string> missileToAircraft_; // �̻��� : Ű , �װ��� : value
	std::thread workThread_;
	std::atomic<unsigned int> mode_;
	EngagableAircraftQueue engagableAircrafts_;
	std::atomic<bool> isRunning_;
	std::atomic<bool> isChanged_;

	void work();
	void mappingMissileToAircraft(std::string& aircraftId, std::string& missileId);
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