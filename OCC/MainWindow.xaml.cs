using System.Net.Http;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace OCC
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private List<ScenarioInfo> loadedScenarios = new();

        private readonly List<(string url, string id)> subsystems = new()
        {
            //("http://localhost:9011", "TCC"),
            //("http://localhost:9012", "MSS"),
            //("http://localhost:9013", "ATS"),
            //("http://localhost:9014", "LCH"),
            ("http://localhost:9015", "MFR")
        };

        public MainWindow()
        {
            InitializeComponent();
        }

        private async void SendStartSignal_Click(object sender, RoutedEventArgs e)
        {
            // 선택되지 않은 경우 에외 처리
            if (ScenarioList.SelectedIndex == -1)
            {
                MessageBox.Show("시작할 시나리오를 먼저 선택하세요.", "선택 필요", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            var selectedScenario = loadedScenarios[ScenarioList.SelectedIndex];
            string scenarioId = selectedScenario.scenario_id;

            foreach (var (url, id) in subsystems)
            {
                await SendStartSignalAsync(url, id, scenarioId);
            }
            //MessageBox.Show("전송 완료!", "OCC", MessageBoxButton.OK, MessageBoxImage.Information);
        }

        private async Task SendStartSignalAsync(string targetUrl, string subsystemId, string scenarioId)
        {
            using var client = new HttpClient();

            var postData = new
            {
                command = "start",
                subsystem_id = subsystemId,
                scenario_id = scenarioId
            };

            var json = JsonConvert.SerializeObject(postData);
            var content = new StringContent(json, Encoding.UTF8, "application/json");

            try
            {
                var response = await client.PostAsync($"{targetUrl}/start", content);
                if (response.IsSuccessStatusCode)
                {
                    MessageBox.Show($"[OCC] {subsystemId} → {scenarioId} 전송 성공");
                }   
                else
                {
                    MessageBox.Show($"[OCC] {subsystemId} → 응답 오류: {(int)response.StatusCode}");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"[OCC] 예외 발생: {ex.Message}");
            }
        }

        public class ScenarioInfo
        {
            public string scenario_id { get; set; }
            public string scenario_title { get; set; }
        }

        private async void LoadScenarios_Click(object sender, RoutedEventArgs e)
        {
            string serverUrl = "http://localhost:8000/scenario/list"; // 시나리오 서버 주소로 교체
            ScenarioList.Items.Clear();
            loadedScenarios.Clear();

            try
            {
                using var client = new HttpClient();
                var response = await client.GetAsync(serverUrl);

                if (!response.IsSuccessStatusCode)
                {
                    MessageBox.Show($"서버 응답 오류: {(int)response.StatusCode}");
                    return;
                }

                string jsonString = await response.Content.ReadAsStringAsync();

                var scenarios = JsonConvert.DeserializeObject<List<ScenarioInfo>>(jsonString);

                if (scenarios != null)
                {
                    loadedScenarios = scenarios;

                    foreach (var scenario in scenarios)
                    {
                        ScenarioList.Items.Add($"[{scenario.scenario_id}] {scenario.scenario_title}");
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"예외 발생: {ex.Message}");
            }
        }
    }
}