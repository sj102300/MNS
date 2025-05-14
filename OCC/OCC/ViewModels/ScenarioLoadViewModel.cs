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
        public ICommand GoBackCommand { get; }
        public ICommand LoadCommand { get; }
        public ICommand StartCommand { get; }
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
                 canExecute: _ => NavigationService?.CanGoBack == true
            );

            //시작 버튼 초기화
            //StartCommand = new RelayCommand<object>(
            //     execute: _ => Start(),
            //     canExecute: _ => NavigationService?.CanGoBack == true
            //);
        }


        private async void Load()
        {
            string serverUrl = "http://127.0.0.1:8000/scenario/list"; // 시나리오 서버 주소로 교체

            try
            {
                // 백그라운드에서 네트워크 요청 및 파싱
                var scenarios = await Task.Run(async () =>
                {
                    using var client = new HttpClient();

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

    }
}
