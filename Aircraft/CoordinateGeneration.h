#pragma once
#include <utility>
#include <string>
#include <vector>
using namespace std;
class CoordinateGeneration {
public:
	virtual ~CoordinateGeneration() {}
	virtual pair<double, double> makePoint(double dx, double dy, pair<double, double> currentPoint) = 0;
	virtual void sendAircraftInfo(pair<double, double> currentPoint, string id, char IFF) = 0;
	virtual vector<vector<string>> scenarioReciever() = 0;
	virtual vector<double> makeStartOpt(std::pair<double, double> startPoint, std::pair<double, double> finishPoint) = 0;
};