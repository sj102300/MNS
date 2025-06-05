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
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Navigation;
using Newtonsoft.Json;
using OCC.Commands;
using OCC.Models;
using OCC.Utils;

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
        public ICommand QuitCommand { get; }

        // 서브시스템 정보
        private readonly List<(string url, string id)> subsystems = new()
        {
            ($"http://192.168.2.66:8080", "TCC"),
            ($"{Network.MFR}", "MFR"),
            ($"{Network.ATS}", "ATS"),
            ($"{Network.LCH}", "LCH"),
            ($"{Network.MSS}", "MSS"),
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

            // 종료 버튼 초기화
            QuitCommand = new RelayCommand<object>(
                 execute: _ => Quit(),
                 canExecute: _ => SelectedScenario != null
            );

            ScenarioCollection = new ObservableCollection<ScenarioGet>();

            // 페이지 진입 시 자동으로 불러오기
            Debug.WriteLine("loadcommand viewmodel execute");
            LoadCommand.Execute(null);
        }

        private async void Load()
        {
            string serverUrl = $"{Network.SCN}/scenario/list"; // 시나리오 서버 주소로 교체
            Debug.WriteLine("불러오기 요청");
            // UI 쓰레드에서 ObservableCollection 업데이트
            Application.Current.Dispatcher.Invoke(() =>
            {
                ScenarioCollection.Clear();
            });

            try
            {
                // 백그라운드에서 네트워크 요청 및 파싱
                var scenarios = await Task.Run(async () =>
                {
                    using var client = new HttpClient();
                    client.Timeout = TimeSpan.FromSeconds(3); // 3초 타임아웃 설정
                    var response = await client.GetAsync(serverUrl);

                    if (!response.IsSuccessStatusCode)
                        throw new Exception($"서버 응답 오류: {(int)response.StatusCode}");

                    string jsonString = await response.Content.ReadAsStringAsync();

                    var parsed = JsonConvert.DeserializeObject<List<ScenarioGet>>(jsonString);

                    return parsed ?? new List<ScenarioGet>();
                });

                // UI 쓰레드에서 ObservableCollection 업데이트
                Application.Current.Dispatcher.Invoke(() =>
                {
                    ScenarioCollection.Clear();
                    foreach (var scenario in scenarios)
                    {
                        ScenarioCollection.Add(scenario);
                    }
                });
            }
            catch (Exception ex)
            {
                MessageBox.Show($"시나리오 목록 불러오기 실패: {ex.Message}", "오류", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void NavigateToAttackDisplayPage()
        {
            // AttackViewModel 하나 생성
            if(NavigationService == null)
            {
                Debug.WriteLine("AttackDisplayPage로 갈때 navigationService가 없습니다!");
            }
            var attackViewModel = new AttackViewModel(NavigationService);
            // 현재 활성 창에 AttackDisplayPage를 띄우고 ViewModel 할당
            var mainWindow = Application.Current.Windows.OfType<MainWindow>().FirstOrDefault();
            if (mainWindow != null)
            {
                var frame = mainWindow.FindName("MainFrame") as Frame;
                if (frame != null)
                {
                    var attackDisplayPage = new OCC.Views.AttackDisplayPage(attackViewModel);
                    frame.Navigate(attackDisplayPage);
                }
            }

            // AircraftLogPage를 Window에 담아서 띄우고 ViewModel 할당
            var aircraftLogPage = new OCC.Views.AircraftLogPage(attackViewModel);
            var aircraftLogWindow = new Window
            {
                Title = "Aircraft Log",
                Width = 600,
                Height = 800,
                Content = aircraftLogPage
            };
            aircraftLogWindow.Show();

            // MissileLogPage를 Window에 담아서 띄우고 ViewModel 할당
            var missileLogPage = new OCC.Views.MissileLogPage(attackViewModel);
            var missileLogWindow = new Window
            {
                Title = "Missile Log",
                Width = 600,
                Height = 800,
                Content = missileLogPage
            };
            missileLogWindow.Show();

        }

        private async void Start()
        {
            if (SelectedScenario == null)
            {
                MessageBox.Show("시작할 시나리오를 먼저 선택하세요.", "선택 필요", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            Debug.WriteLine("디버깅 용. ...");
           //NavigateToAttackDisplayPage();

#if true
            string scenarioId = SelectedScenario.scenario_id;

            // 모든 서브시스템에 대해 비동기 요청을 병렬로 실행
            var tasks = subsystems.Select(subsystem =>
                SendStartSignalAsync(subsystem.url, subsystem.id, scenarioId)
            ).ToList();

            var results = await Task.WhenAll(tasks);

            // 결과 집계
            if (results.All(r => r))
            {
                // 모두 성공
                var result = MessageBox.Show(
                    $"[OCC] 모든 서브시스템에 시나리오({scenarioId}) 시작 신호 전송 완료",
                    "시작 완료",
                    MessageBoxButton.OK,
                    MessageBoxImage.Information
                );
                if (result == MessageBoxResult.OK)
                {
                    NavigateToAttackDisplayPage();
                }
            }
            else
            {
                // 일부 실패
                MessageBox.Show(
                    $"[OCC] 일부 서브시스템에 시작 신호 전송 실패\n상세 오류는 개별 메시지를 확인하세요.",
                    "시작 실패",
                    MessageBoxButton.OK,
                    MessageBoxImage.Warning
                );
            }
#endif
        }
        private async Task<bool> SendStartSignalAsync(string targetUrl, string subsystemId, string scenarioId)
        {
            using var client = new HttpClient();
            client.Timeout = TimeSpan.FromSeconds(3);
            var postData = new
            {
                command = "start",
                subsystem_id = subsystemId,
                scenario_id = scenarioId
            };

            var json = JsonConvert.SerializeObject(postData);
            var content = new StringContent(json, Encoding.UTF8, "application/json");

            Debug.WriteLine($"post data : {json}");
            try
            {
                var response = await client.PostAsync($"{targetUrl}/start", content);
                Debug.WriteLine($"response : {response.ToString()}");
                if (response.IsSuccessStatusCode)
                {
                    // 성공
                    return true;
                }
                else
                {
                    //MessageBox.Show($"[OCC] {subsystemId} → 응답 오류: {(int)response.StatusCode}");
                    return false;
                }
            }
            catch (Exception ex)
            {
                //MessageBox.Show($"[OCC] {subsystemId} 예외 발생: {ex.Message}");
                return false;
            }
        }

        private async void Quit()
        {
            Debug.WriteLine("종료 요청");
            // 모든 서브시스템에 대해 비동기 요청을 병렬로 실행
            var tasks = subsystems.Select(subsystem =>
                SendQuitSignalAsync(subsystem.url, subsystem.id)
            ).ToList();

            var results = await Task.WhenAll(tasks);

            // 결과 집계
            if (results.All(r => r))
            {
                // 모두 성공
                var result = MessageBox.Show(
                    $"[OCC] 모든 서브시스템에 시나리오 종료 전송 완료",
                    "종료 완료",
                    MessageBoxButton.OK,
                    MessageBoxImage.Information
                );
                if (result == MessageBoxResult.OK)
                {
                    //NavigationService?.GoBack();
                    //NavigateToScenarioCreate();
                }
            }
            else
            {
                // 일부 실패
                MessageBox.Show(
                    $"[OCC] 일부 서브시스템에 시작 신호 전송 실패\n상세 오류는 개별 메시지를 확인하세요.",
                    "시작 실패",
                    MessageBoxButton.OK,
                    MessageBoxImage.Warning
                );
            }
        }

        private async Task<bool> SendQuitSignalAsync(string targetUrl, string subsystemId)
        {
            using var client = new HttpClient();
            client.Timeout = TimeSpan.FromSeconds(3); // 3초 타임아웃 설정

            try
            {
                var response = await client.GetAsync($"{targetUrl}/quit");
                Debug.WriteLine($"response : {response.ToString()}");
                if (response.IsSuccessStatusCode)
                {
                    return true;
                }
                else
                {
                    //MessageBox.Show($"[OCC] {subsystemId} → 응답 오류: {(int)response.StatusCode}");
                    return false;
                }
            }
            catch (Exception ex)
            {
                //MessageBox.Show($"[OCC] {subsystemId}예외 발생: {ex.Message}");
                return false;
            }
        }

    }
}
