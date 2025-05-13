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

    //��ɾ �����ڵ�� ��ȯ
    wstring wcommand = stringToWString(command);

    // ���μ��� ����
    if (!CreateProcess(NULL,   // No module name (use command line)
        &wcommand[0],        // Command line
        NULL,       // Process handle not inheritable
        NULL,       // Thread handle not inheritable
        FALSE,      // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,          // ���ο� �ܼ�â ������ �÷���
        NULL,       // Use parent's environment block
        NULL,       // Use parent's starting directory 
        &si,        // Pointer to STARTUPINFO structure
        &pi)       // Pointer to PROCESS_INFORMATION structure
        ) {
        cout << "CreateProcess failed (" << GetLastError() << ").\n";
        return;
    }

    // ���μ����� ����� ������ ��ٸ��� �ʰ� �������� �Ѿ
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

    // ���̵����ʹ� ������ http�� ���� �ó������� ���� ���Ź޾ƾ� ��
    // �Ǿƽĺ� �����͵� ó���ؾ� �� + �װ��� ���� ������
    // ���̵�����1
    s_x.push_back(0);
    s_y.push_back(0);
    z.push_back(10);
    f_x.push_back(90);
    f_y.push_back(180);
    IFF.push_back('O');
    id.push_back("ATS-0001");

    // ���̵�����2
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

        // �� ����� ������ ���μ����� ����
        makeCommand(command);
    }

    return 0;
}