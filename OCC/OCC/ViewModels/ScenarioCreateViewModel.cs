using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Net.Http;
using System.Text;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Navigation;
using OCC.Commands;
using OCC.Models;
using OCC.Utils;

namespace OCC.ViewModels
{
    public class ScenarioCreateViewModel : BaseViewModel
    {
        private const int MaxAircraftCount = 6; // 아군/적군 최대 사이클 수
        private const int MaxBatteryCount = 1; // 포대 최대 개수
        private string? _selectedItem; // 선택된 아이템
        private Coordinate? _startPoint; // 시작점
        private Coordinate? _endPoint; // 끝점
        private int _aircraftCycleCount = 0; // 현재 아군/적군 사이클 수
        private int _batteryCount = 0; // 현재 포대 개수

        public ObservableCollection<string> Items { get; set; } // 콤보박스 항목
        public ObservableCollection<Aircraft> AircraftList { get; set; } // 항공기 리스트
        public ScenarioPost ScenarioData { get; set; } // 시나리오 데이터

        public Coordinate? BatteryLocation
        {
            get => ScenarioData.battery_location;
            set
            {
                ScenarioData.battery_location = value;
                OnPropertyChanged();
            }
        }

        public Coordinate? StartPoint
        {
            get => _startPoint;
            set
            {
                if (SetProperty(ref _startPoint, value))
                {
                    OnPropertyChanged(nameof(StartPoint));
                }
            }
        }

        public Coordinate? EndPoint
        {
            get => _endPoint;
            set
            {
                if (SetProperty(ref _endPoint, value))
                {
                    OnPropertyChanged(nameof(EndPoint));
                }
            }
        }

        public string? SelectedItem
        {
            get => _selectedItem;
            set
            {
                if (SetProperty(ref _selectedItem, value))
                {
                    // 선택된 항목이 변경되었을 때 초기화
                    _startPoint = null;
                    _endPoint = null;
                }
            }
        }

        public ICommand MapClickCommand { get; set; } // 지도 클릭 명령
        public ICommand SaveCommand { get; set; } // 저장 명령
        public ICommand ExportToJsonCommand { get; set; } // JSON 내보내기 명령
        public ICommand GoBackCommand { get; }

        public ScenarioCreateViewModel()
        {
            // 콤보박스 항목 초기화
            Items = new ObservableCollection<string>
            {
                "적군",
                "아군",
                "포대"
            };

            // 디폴트값을 "적군"으로 설정
            SelectedItem = "적군";

            // 시나리오 데이터 초기화
            ScenarioData = new ScenarioPost
            {
                scenario_id = Guid.NewGuid().ToString(),
                scenario_title = "새 시나리오",
                aircraft_list = new List<Aircraft>(),
                battery_location = null
            };

            AircraftList = new ObservableCollection<Aircraft>();

            // 지도 클릭 명령 초기화
            MapClickCommand = new RelayCommand<Coordinate>(
                execute: point => HandleMapClick(point)
            );

            // 저장 명령 초기화
            SaveCommand = new RelayCommand<object>(
                execute: async _ => await SaveScenarioAsync(),
                canExecute: _ => CanSaveScenario()
            );

            // 뒤로가기 버튼 초기화
            GoBackCommand = new RelayCommand<object>(
                 execute: _ => GoBack(),
                 canExecute: _ => NavigationService?.CanGoBack == true
            );
        }

        private bool CanSaveScenario()
        {
            return _aircraftCycleCount > 0 && _batteryCount > 0;
        }

        public bool HandleMapClick(Coordinate point)
        {
            Debug.WriteLine($"HandleMapClick 호출됨: SelectedItem={SelectedItem}, StartPoint={_startPoint}, EndPoint={_endPoint}");
            if (SelectedItem == "아군" || SelectedItem == "적군")
            {
                // 아군/적군 점 찍기 로직
                if (_aircraftCycleCount >= MaxAircraftCount)
                {
                    MessageBox.Show("아군과 적군은 최대 6개만 추가할 수 있습니다.", "경고", MessageBoxButton.OK, MessageBoxImage.Warning);
                    return false; // 점을 찍지 않음
                }

                if (_startPoint == null)
                {
                    _startPoint = point;
                    Debug.WriteLine($"시작점 설정: {_startPoint.latitude}, {_startPoint.longitude}");
                }
                else if (_endPoint == null)
                {
                    _endPoint = point;
                    Debug.WriteLine($"끝점 설정: {_endPoint.latitude}, {_endPoint.longitude}");

                    // 사이클 완료
                    _aircraftCycleCount++;
                    Debug.WriteLine($"현재 아군/적군 사이클 수: {_aircraftCycleCount}");

                    // 항공기 데이터 추가
                    var aircraft = new Aircraft
                    {
                        aircraft_id = "ATS-" + _aircraftCycleCount.ToString("D4"),
                        friend_or_foe = SelectedItem == "아군" ? "O" : "E",
                        start_point = _startPoint,
                        end_point = _endPoint
                    };

                    ScenarioData.aircraft_list.Add(aircraft);
                    AircraftList.Add(aircraft);

                    // 시작점과 끝점 초기화
                    _startPoint = null;
                    _endPoint = null;
                }
            }
            else if (SelectedItem == "포대")
            {
                // 포대 점 찍기 로직
                if (_batteryCount >= MaxBatteryCount)
                {
                    MessageBox.Show("포대는 최대 1개만 추가할 수 있습니다.", "경고", MessageBoxButton.OK, MessageBoxImage.Warning);
                    return false; // 점을 찍지 않음
                }

                BatteryLocation = point;
                _batteryCount++;
                Debug.WriteLine($"포대 위치 설정: {BatteryLocation.latitude}, {BatteryLocation.longitude}");
            }

            return true; // 점을 찍음
        }

        public Brush GetMarkerColor()
        {
            return SelectedItem switch
            {
                "아군" => Brushes.Blue,
                "적군" => Brushes.Red,
                "포대" => Brushes.Green,
                _ => Brushes.Black
            };
        }

        private async Task SaveScenarioAsync()
        {
            try
            {
                // 시나리오 제목 입력 메시지 창 표시
                string scenarioTitle = PromptForScenarioTitle();
                if (string.IsNullOrWhiteSpace(scenarioTitle))
                {
                    MessageBox.Show("시나리오 제목을 입력해야 합니다.", "경고", MessageBoxButton.OK, MessageBoxImage.Warning);
                    return;
                }

                // 시나리오 제목 설정
                ScenarioData.scenario_title = scenarioTitle;

                // AircraftCount 업데이트
                ScenarioData.aircraft_count = ScenarioData.aircraft_list.Count;

                // JSON 변환
                string json = JsonSerializer.Serialize(ScenarioData, new JsonSerializerOptions
                {
                    WriteIndented = true,
                    Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping // UTF-8 인코딩 설정(한국어 깨지는 문제 해결)
                });

                // JSON 출력 (디버깅용)
                Debug.WriteLine(json);

                // 서버에 POST 요청
                Debug.WriteLine("저장 요청");
                using var client = new HttpClient();
                client.Timeout = TimeSpan.FromSeconds(3); // 3초 타임아웃 설정
                var content = new StringContent(json, Encoding.UTF8, "application/json");

                string serverUrl = $"{Network.SCN}/scenario/save";
                var response = await client.PostAsync(serverUrl, content);
                Debug.WriteLine("저장 응답");
                if (response.IsSuccessStatusCode)
                {
                    MessageBox.Show("시나리오가 성공적으로 저장되었습니다.", "정보", MessageBoxButton.OK, MessageBoxImage.Information);
                    GoBack();
                }
                else
                {
                    MessageBox.Show($"서버 오류: {response.StatusCode}", "오류", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"저장 중 오류가 발생했습니다: {ex.Message}", "오류", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private string PromptForScenarioTitle()
        {
            // 메시지 창 생성
            var inputDialog = new Window
            {
                Title = "시나리오 제목 입력",
                Width = 400,
                Height = 200,
                WindowStartupLocation = WindowStartupLocation.CenterScreen,
                ResizeMode = ResizeMode.NoResize
            };

            // 레이아웃 설정
            var stackPanel = new StackPanel { Margin = new Thickness(20) };
            var textBlock = new TextBlock { Text = "시나리오 제목을 입력하세요:", Margin = new Thickness(0, 0, 0, 10) };

            // TextBox 설정: 부모 컨테이너에 맞게 확장
            var textBox = new TextBox
            {
                HorizontalAlignment = HorizontalAlignment.Stretch, // 부모 컨테이너에 맞게 확장
                VerticalAlignment = VerticalAlignment.Center,
                Margin = new Thickness(0, 0, 0, 10) // 상하좌우 여백 설정
            };
            // 버튼 패널 설정
            var buttonPanel = new StackPanel
            {
                Orientation = Orientation.Horizontal,
                HorizontalAlignment = HorizontalAlignment.Right
            };
            var okButton = new Button { Content = "확인", Width = 75, Margin = new Thickness(5) };
            var cancelButton = new Button { Content = "취소", Width = 75, Margin = new Thickness(5) };

            // 버튼 클릭 이벤트
            okButton.Click += (s, e) => inputDialog.DialogResult = true;
            cancelButton.Click += (s, e) => inputDialog.DialogResult = false;

            // 레이아웃 구성
            buttonPanel.Children.Add(okButton);
            buttonPanel.Children.Add(cancelButton);
            stackPanel.Children.Add(textBlock);
            stackPanel.Children.Add(textBox);
            stackPanel.Children.Add(buttonPanel);
            inputDialog.Content = stackPanel;

            // 메시지 창 표시
            if (inputDialog.ShowDialog() == true)
            {
                return textBox.Text; // 입력된 제목 반환
            }

            return string.Empty; // 취소 시 빈 문자열 반환
        }
    }
}