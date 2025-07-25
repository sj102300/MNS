#pragma once
#include "share.h"
#include <string>

//미사일로그창에서,, 유도중으로 바뀌게,, 메시지? 글자

class Missile {
public:
	Missile(std::string id, TCC::Position pos_, int status, TCC::Position impact);

	enum MissileStatus : unsigned int {
		Wait = 0,   // 대기(발사 전)
		Fly = 1,      // 비행(표적을 향해 비행 중)
		Splashed = 2,        // 격추(미사일이 표적 격추)
		EmergencyDestroyed = 3,        // 비상폭파(사용자가 비상폭파 명령 내림)
		SelfDestroyed = 4,        // 자폭 (IP지났을 때 자동 폭파)
		FollowUp = 5,				//유도 중
		LaunchRequest = 6,			//발사 요청 중
	};

	bool checkId(std::string id);
	bool isAvailable(unsigned int status, std::string& missileId);
	void updateStatus(MissileStatus);
	unsigned int getMissileStatus();
	~Missile();

private:
	/*typedef struct _direction_vector {
		double dx_;
		double dy_;
		bool isZeroVector() {
			return dx_ == 0 && dy_ == 0;
		}
	} DirVector;*/

	std::string missileId_;
	TCC::Position position_;
	MissileStatus status_;
	TCC::Position impactPoint_;
};