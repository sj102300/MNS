[HTTP API] - SCN/main.cpp 에 정의

const std::string SCENARIO_DIR = "./Scenarios";
const utility::string_t API_SCENARIO_DETAIL = U("http://127.0.0.1:8000/scenario/detail");
const utility::string_t API_SCENARIO_LIST = U("http://127.0.0.1:8000/scenario/list");



[HTTP API] - OCC/MainWindow.xaml.cs 에 정의

private readonly List<(string url, string id)> subsystems = new()
{
	//("http://127.0.0.1:9011", "TCC"),
	//("http://127.0.0.1:9012", "MSS"),
	//("http://127.0.0.1:9013", "ATS"),
	//("http://127.0.0.1:9014", "LCH"),
	("http://127.0.0.1:9015", "MFR")
};
...
var response = await client.PostAsync($"{targetUrl}/start", content);
...



HTTP 서버
SCN: 192.168.1.1:8080
ATS: 192.168.1.2:8080
MFR: 192.168.1.3:8080
LCH: 192.168.1.4:8080
MSS: 192.168.1.5:8080
TCC: 192.168.1.6:8080
OCC: 192.168.1.7:8080

TCP
TCC: 192.168.2.6:9000(명령 정보(수동발사 명령, 모드 전환, 비상폭파))
OCC: 192.168.2.7:9000(항공기정보)
192.168.2.7:9001(미사일정보)

UDP
시뮬레이션 망: 239.0.0.1
항공기망: 239.0.0.2
미사일망: 239.0.0.3
포트번호는 IDD참고

