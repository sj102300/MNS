#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

int main(void) {


	const char* json = "{\"name\":\"Alice\",\"age\":250}";

	Document doc;
	doc.Parse(json);

	if (doc.HasMember("name") && doc["name"].IsString()) {
		cout << doc["name"].GetString() << endl;
	}
	if (doc.HasMember("age") && doc["age"].IsInt()) {
		cout << doc["age"].GetInt() << endl;
	}

	return 0;
}