#pragma once

#include <string>
#include <vector>

using namespace std;
#pragma pack(push, 1)
struct Location {
	double latitude;
	double longitude;
	double altitude;
};
#pragma pack(pop)

struct Aircraft {
	string id;
	Location start_point;
	Location end_point;
	char friend_or_foe;
};


struct Scenario {
	string scenario_id;
	string title;
	int aircraft_count;
	vector<Aircraft> aircraft_list;
	Location battery_location;
};

Scenario parseScenarioFromFile(const string& filename);