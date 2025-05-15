using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Net.Http;
using System.Runtime.InteropServices.Marshalling;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Navigation;
using Newtonsoft.Json;
using OCC.Commands;
using OCC.Models;
using static OCC.MainWindow;

namespace OCC.ViewModels
{
    public class ScenarioLoadViewModel : BaseViewModel
    {
        public ObservableCollection<ScenarioGet> ScenarioCollection { get; set; }

        private ScenarioGet _selectedScenario;
        public ScenarioGet SelectedScenario
        {
            get => _selectedScenario;
            set => SetProperty(ref _selectedScenario, value);
        }

        public ICommand GoBackCommand { get; }
        public ICommand LoadCommand { get; }
        public ICommand StartCommand { get; }

        // 서브시스템 정보
        private readonly List<(string url, string id)> subsystems = new()
        {
            //("http://127.0.0.1:9011", "TCC"),
            //("http://127.0.0.1:9012", "MSS"),
            //("http://127.0.0.1:9013", "ATS"),
            //("http://127.0.0.1:9014", "LCH"),
            ("http://127.0.0.1:9015", "MFR")
        };

        public ScenarioLoadViewModel()
        {
            //뒤로가기 버튼 초기화
            GoBackCommand = new RelayCommand<object>(
                 execute: _ => GoBack(),
                 canExecute: _ => NavigationService?.CanGoBack == true
            );

            //불러오기 버튼 초기화
            LoadCommand = new RelayCommand<object>(
                 execute: _ => Load(),
                 canExecute: _ => true
            );

            // 시작 버튼 초기화
            StartCommand = new RelayCommand<object>(
                 execute: _ => Start(),
                 canExecute: _ => SelectedScenario != null
            );

            ScenarioCollection = new ObservableCollection<ScenarioGet>();

        }


        private async void Load()
        {
            string serverUrl = "http://127.0.0.1:8000/scenario/list"; // 시나리오 서버 주소로 교체

            // UI 쓰레드에서 ObservableCollection 업데이트
            Application.Current.Dispatcher.Invoke(() =>
            {
                ScenarioCollection.Clear();

                // 더미 데이터 추가
                ScenarioCollection.Add(new ScenarioGet { ScenarioId = "SCENE-01", ScenarioTitle = "더미 시나리오 1" });
                ScenarioCollection.Add(new ScenarioGet { ScenarioId = "SCENE-02", ScenarioTitle = "더미 시나리오 2" });
                ScenarioCollection.Add(new ScenarioGet { ScenarioId = "SCENE-03", ScenarioTitle = "더미 시나리오 3" });
                ScenarioCollection.Add(new ScenarioGet { ScenarioId = "SCENE-04", ScenarioTitle = "더미 시나리오 4" });
                ScenarioCollection.Add(new ScenarioGet { ScenarioId = "SCENE-05", ScenarioTitle = "더미 시나리오 5" });
                ScenarioCollection.Add(new ScenarioGet { ScenarioId = "SCENE-06", ScenarioTitle = "더미 시나리오 6" });
            });

            //try
            //{
            //    // 백그라운드에서 네트워크 요청 및 파싱
            //    var scenarios = await Task.Run(async () =>
            //    {
            //        using var client = new HttpClient();

            //        var response = await client.GetAsync(serverUrl);

            //        if (!response.IsSuccessStatusCode)
            //            throw new Exception($"서버 응답 오류: {(int)response.StatusCode}");

            //        string jsonString = await response.Content.ReadAsStringAsync();

            //        var parsed = JsonConvert.DeserializeObject<List<ScenarioGet>>(jsonString);

            //        return parsed ?? new List<ScenarioGet>();
            //    });

            //    // UI 쓰레드에서 ObservableCollection 업데이트
            //    Application.Current.Dispatcher.Invoke(() =>
            //    {
            //        ScenarioCollection.Clear();
            //        foreach (var scenario in scenarios)
            //        {
            //            ScenarioCollection.Add(scenario);
            //        }

            //        // 더미 데이터 추가
            //        ScenarioCollection.Add(new ScenarioGet { ScenarioId = "SCENE-01", ScenarioTitle = "더미 시나리오 1" });
            //        ScenarioCollection.Add(new ScenarioGet { ScenarioId = "SCENE-02", ScenarioTitle = "더미 시나리오 2" });
            //    });
            //}
            //catch (Exception ex)
            //{
            //    MessageBox.Show($"시나리오 목록 불러오기 실패: {ex.Message}", "오류", MessageBoxButton.OK, MessageBoxImage.Error);
            //}
        }

        private async void Start()
        {
            if (SelectedScenario == null)
            {
                MessageBox.Show("시작할 시나리오를 먼저 선택하세요.", "선택 필요", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            string scenarioId = SelectedScenario.ScenarioId;

            foreach (var (url, id) in subsystems)
            {
                await SendStartSignalAsync(url, id, scenarioId);
            }
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

    }
}
