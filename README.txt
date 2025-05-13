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
