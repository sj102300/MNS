#pragma once

//#include "CommandReceiver.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <queue>
#include <thread>
#include <atomic>

#include <chrono>
#include <iomanip>
#include <sstream>

#include "UdpSender.h"
#include "UdpMulticastSender.h"

class MissileManager;
class AircraftManager;

class EngagementManager {
public:
	enum DestroyType {
		SelfDestroy = 10,
		EmergencyDestroy = 20,
		EngagementSuccess = 30,
		WDL = 40,
	};
	EngagementManager();
	void start();
	void stop();
	bool init(TCC::UdpSender* sender, AircraftManager* aircraftManager, TCC::UdpMulticastSender* multisender, MissileManager* missileManager, TCC::Position &batteryLoc);
	bool engagementSuccess(std::string targetAircraftId, std::string targetMissileId);
	bool isHitTarget(std::string& missileId);
	unsigned int changeMode(unsigned int mode);
	bool launchMissile(std::string& commandId, std::string& aircraftId);
	bool emergencyDestroy(std::string commandId, std::string missileId);
	void addEngagableAircraft(std::string& aircraftId);
	bool manualFire(std::string commandId, std::string targetAircraftId);
	bool handleMissileDestroyed(std::string& missileId, unsigned int type);
	bool weaponDataLink(std::string commandId, std::string aircraftId, std::string missileId);

	void notifyThread();
	~EngagementManager();

private:
	enum Mode {
		Auto = 0,
		Manual = 1,
	};
	

	class EngagableAircraftQueue {
	public:
		std::mutex mtx_;		//queue용
		std::queue<std::string> queue;
		std::unordered_set<std::string> set;

		bool pushQueue(std::string& aircraftId);
		bool popQueue(std::string& aircraftId);
		bool empty();
		void clear();
	};

	std::mutex mtx_;				//cv_용
	std::atomic<bool> isRunning_;
	std::atomic<bool> isChanged_;
	std::condition_variable cv_;

	TCC::UdpSender* sender_;
	TCC::UdpMulticastSender * multisender_;
	MissileManager* missileManager_;
	AircraftManager* aircraftManager_;
	std::unordered_map<std::string, std::string> missileToAircraft_; // 미사일Id : 항공기 Id
	std::unordered_map<std::string, std::pair<std::string, std::string>> fireCommands_;		// 발사명령 식별자 : {미사일Id, 항공기Id}
	std::thread workThread_;
	TCC::Position batteryLoc_;

	std::atomic<unsigned int> mode_;
	EngagableAircraftQueue engagableAircrafts_;

	//필요시 EngagableAircraftQueue같은 자료구조 사용
	bool isEmergencySuccess; // cv로 알림 받을 변수
	std::string missileId; // cv로 알림 받을 변수 

	void work();
	bool mappingMissileToAircraft(std::string& aircraftId, std::string& missileId);
	void makeAutoFireCommandId(std::string& commandId);
	void makeWDLCommandId(std::string& commandId);

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