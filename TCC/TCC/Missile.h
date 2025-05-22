#pragma once
#include "share.h"
#include <string>

class Missile {
private:
	/*typedef struct _direction_vector {
		double dx_;
		double dy_;
		bool isZeroVector() {
			return dx_ == 0 && dy_ == 0;
		}
	} DirVector;*/

	void updateStatus(TCC::MissileStatus);

	std::string missileId_;
	TCC::Position position_;
	TCC::MissileStatus status_;
	TCC::Position impactPoint_;

public:
	Missile(std::string id, TCC::Position pos_, int status, TCC::Position impact);

	//MissileManager가 데이터 필요할 때 사용할 getter
	std::string getId() const;
	TCC::Position getPosition() const;
	TCC::MissileStatus getStatus() const;
	TCC::Position getImpactPoint() const;
};