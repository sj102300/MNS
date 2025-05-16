
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

//AircraftReceiver�� �װ��� �߰�, �װ��� ���� ������Ʈ
//SucessReceiver�� �װ��� ���� ������Ʈ �س��� - 

//������ �̻����� ���������� ���,
