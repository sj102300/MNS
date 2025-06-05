using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http;
using System.Net.Sockets;
using System.Reflection.PortableExecutable;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using Newtonsoft.Json;
using OCC.Commands;
using OCC.Models;
using OCC.Utils;

namespace OCC.ViewModels
{
    public class AttackDisplayViewModel : BaseViewModel
    {

        // ICommand Interface 구현
        public ICommand ManualFireCommand { get; }
        public ICommand EmergencyDestroyCommand { get; }
        public ICommand QuitScenarioCommand { get; }
        public ICommand ChangeFireModeCommand { get; }
        public ICommand QuitCommand { get; }

        private readonly string _udpHost = "192.168.2.189";
        private readonly int _udpPort = 9999;

        public AttackDisplayViewModel()
        {
            //// 수동 발사 명령
            //ManualFireCommand = new RelayCommand<object>(
            //    execute: async _ => SendManualFire(),
            //    canExecute: _ => true
            //);

            //// 비상폭파 명령
            //EmergencyDestroyCommand = new RelayCommand<object>(
            //     execute: _ => sendEmergencyDestroy(),
            //     canExecute: _ => true
            //);

            //// 발사 모드 변경 명령
            //ChangeFireModeCommand = new RelayCommand<object>(
            //    execute: param => ChangeFireMode(param),
            //    canExecute: _ => true
            //);

            // 종료 버튼 초기화
            QuitCommand = new RelayCommand<object>(
                 execute: _ => Quit(),
                 canExecute: _ => true
            );
        }

        public async Task<Coordinate?> GetScenarioInfoAsync()
        {
            try
            {
                using var client = new HttpClient();
                client.Timeout = TimeSpan.FromSeconds(3);
                // 실제 SCN API 주소와 파라미터로 교체
                string url = $"{Network.SCN}/scenario/info"; // 예시
                var response = await client.GetAsync(url);
                if (!response.IsSuccessStatusCode)
                    return null;

                string json = await response.Content.ReadAsStringAsync();
                // 예시: Coordinate 객체로 역직렬화
                var coord = JsonConvert.DeserializeObject<Coordinate>(json);
                return coord;
            }
            catch
            {
                return null;
            }
        }

        // 서브시스템 정보
        private readonly List<(string url, string id)> subsystems = new()
        {
            ("http://192.168.2.66:8080", "TCC"),
            //($"{Network.TCC}", "TCC"),
            //($"{Network.MSS}", "MSS"),
            //($"{Network.ATS}", "ATS"),
            //($"{Network.LCH}", "LCH"),
            //($"{Network.MFR}", "MFR")
        };

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
                //MessageBox.Show($"[OCC] 예외 발생: {ex.Message}");
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

    }
}