#include "ScenarioParser.h"
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

using namespace rapidjson;
using namespace std;

Scenario parseScenarioFromFile(const string& filename) {
	ifstream ifs(filename);
	IStreamWrapper isw(ifs);
	Document doc;
	doc.ParseStream(isw);

	Scenario scenario;
	scenario.scenario_id = doc["scenario_id"].GetString();
	scenario.title = doc["scenario_title"].GetString();
	scenario.aircraft_count = doc["aircraft_count"].GetInt();

	const auto& aircrafts = doc["aircraft_list"];
	for (auto& a : aircrafts.GetArray()) {
		Aircraft ac;
		ac.id = a["aircraft_id"].GetString();
		ac.start_point = {
		   a["start_point"]["latitude"].GetDouble(),
		   a["start_point"]["longitude"].GetDouble(),
		   a["start_point"]["altitude"].GetDouble()
		};
		ac.end_point = {
			a["end_point"]["latitude"].GetDouble(),
			a["end_point"]["longitude"].GetDouble(),
			a["end_point"]["altitude"].GetDouble()
		};
		ac.friend_or_foe = a["friend_or_foe"].GetString()[0];
		scenario.aircraft_list.push_back(ac);
	}
	scenario.battery_location = {
		doc["battery_location"]["latitude"].GetDouble(),
		doc["battery_location"]["longitude"].GetDouble(),
		doc["battery_location"]["altitude"].GetDouble()
	};

	return scenario;
}