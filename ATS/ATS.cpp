#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ATS.h"
#include <windows.h>
#include <iostream>
#include <sstream>

using namespace sm;

void ATS::setAircraftList(const std::vector<AircraftInfo>& list) {
    aircrafts_ = list;
}

std::string ATS::makeCommandString(const AircraftInfo& info) {
    const Coordinate& s = info.start_point;
    const Coordinate& e = info.end_point;

    std::ostringstream oss;
    oss << "\"C:\\Users\\user\\Desktop\\MNS\\ATS\\x64\\Release\\Aircraft.exe\" "
        << info.aircraft_id << " "
        << s.latitude << " " << s.longitude << " " << s.altitude << " "
        << e.latitude << " " << e.longitude << " "
        << info.friend_or_foe;

    return oss.str();
}

void ATS::makeCommand(const std::string& command) {
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::wstring wcommand(command.begin(), command.end());

    if (!CreateProcessW(NULL,
        &wcommand[0],
        NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE,
        NULL, NULL,
        &si, &pi)) {
        std::cerr << u8"[ATS] CreateProcess failed (" << GetLastError() << ")\n";
        return;
    }

    std::cout << u8"[ATS] ����� �� " << command << "\n";

    runningProccessMember.push_back(pi.hProcess);

    CloseHandle(pi.hThread);
}

void ATS::launchAll() {
    for (const auto& info : aircrafts_) {
        std::string command = makeCommandString(info);
        makeCommand(command);
    }
}

// SCN���� ���� �ñ׳��� ���� ��� ATS���μ����� �����ϴ� �ڵ� �ʿ�
void ATS::terminateAll() {
    for (HANDLE hProcess : runningProccessMember) {
        if (hProcess != NULL) {
            TerminateProcess(hProcess, 0); // ���� ����
            CloseHandle(hProcess);         // �ڵ� ����
        }
    }
    runningProccessMember.clear();
}