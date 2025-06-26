#pragma once

#include "JsonParser.h"
#include <vector>
#include <string>
#include <windows.h>

using namespace sm;

class ATS {
public:
    void setAircraftList(const std::vector<AircraftInfo>& list);
    void launchAll();
    void terminateAll();

private:
    std::vector<AircraftInfo> aircrafts_;
    std::vector<HANDLE> runningProccessMember;
    std::string makeCommandString(const AircraftInfo& info);
    void makeCommand(const std::string& command);  // CreateProcess ·¡ÆÛ
};
