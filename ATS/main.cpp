#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <windows.h>
using namespace std;

wstring stringToWString(const string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    wstring r(buf);
    delete[] buf;
    return r;
}

void makeCommand(const string& command) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    //명령어를 유니코드로 변환
    wstring wcommand = stringToWString(command);

    // 프로세스 생성
    if (!CreateProcess(NULL,   // No module name (use command line)
        &wcommand[0],        // Command line
        NULL,       // Process handle not inheritable
        NULL,       // Thread handle not inheritable
        FALSE,      // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,          // 새로운 콘솔창 만들라는 플래그
        NULL,       // Use parent's environment block
        NULL,       // Use parent's starting directory 
        &si,        // Pointer to STARTUPINFO structure
        &pi)       // Pointer to PROCESS_INFORMATION structure
        ) {
        cout << "CreateProcess failed (" << GetLastError() << ").\n";
        return;
    }

    // 프로세스가 종료될 때까지 기다리지 않고 다음으로 넘어감
    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main(void) {
    vector<double> s_x;
    vector<double> s_y;
    vector<double> z;
    vector<double> f_x;
    vector<double> f_y;
    vector<string> id;
    vector<char> IFF;

    // 더미데이터는 실제로 http를 통해 시나리오로 부터 수신받아야 함
    // 피아식별 데이터도 처리해야 함 + 항공기 개수 데이터
    // 더미데이터1
    s_x.push_back(0);
    s_y.push_back(0);
    z.push_back(10);
    f_x.push_back(90);
    f_y.push_back(180);
    IFF.push_back('O');
    id.push_back("ATS-0001");

    // 더미데이터2
    s_x.push_back(0);
    s_y.push_back(0);
    z.push_back(10);
    f_x.push_back(-90);
    f_y.push_back(-180);
    IFF.push_back('E');
    id.push_back("ATS-0002");
    
    for (int i = 0; i < id.size(); i++) {
        string command = "\"C:\\Users\\user\\Desktop\\MNS\\ATS\\x64\\Debug\\Aircraft.exe\" "
            + id[i] + " " + to_string(s_x[i]) + " " + to_string(s_y[i]) + " " 
            + to_string(z[i]) + " " + to_string(f_x[i]) + " " + to_string(f_y[i]) + " " + IFF[i];

        // 각 명령을 별도의 프로세스로 실행
        makeCommand(command);
    }

    return 0;
}