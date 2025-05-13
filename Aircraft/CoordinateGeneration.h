#pragma once
#include <utility>
#include <string>
using namespace std;
class CoordinateGeneration {
public:
	virtual ~CoordinateGeneration() {}
	virtual void makePoint(pair<double, double> startPoint, pair<double, double> finishPoint) = 0;
	virtual void sendAircraftInfo(pair<double, double> currentPoint, string id, char IFF) = 0;
	virtual void scenarioReciever() = 0;
};