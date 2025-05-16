
#include <string>
#include "share.h"

class IUpdate {  //
public:
	virtual bool updateAircraftPosition(std::string aircraftId, TCC::Position pos) = 0;
};

class AircraftManager : public IUpdate {
public:

private:
	bool updateAircraftPosition(std::string aircraftId, TCC::Position pos) override;
};

//AircraftReceiver가 항공기 추가, 항공기 정보 업데이트
//SucessReceiver가 항공기 정보 업데이트 해놓기 - 

//자폭은 미사일의 상태정보를 듣고,
