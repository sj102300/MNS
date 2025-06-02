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
using OCC.Commands;
using OCC.Utils;

namespace OCC.ViewModels
{
    public class AttackDisplayViewModel : BaseViewModel
    {
        // 발사 모드 enum
        public enum FireModeType { Auto, Manual }

        private FireModeType _fireMode = FireModeType.Auto;
        public FireModeType FireMode
        {
            get => _fireMode;
            set
            {
                if (SetProperty(ref _fireMode, value))
                {
                    OnPropertyChanged(nameof(IsAutoFireMode));
                    OnPropertyChanged(nameof(IsManualFireMode));
                }
            }
        }

        // 라디오버튼 바인딩용
        public bool IsAutoFireMode
        {
            get => FireMode == FireModeType.Auto;
            set
            {
                if (value) FireMode = FireModeType.Auto;
            }
        }
        public bool IsManualFireMode
        {
            get => FireMode == FireModeType.Manual;
            set
            {
                if (value) FireMode = FireModeType.Manual;
            }
        }

        // ICommand Interface 구현
        public ICommand ManualFireCommand { get; }
        public ICommand EmergencyDestroyCommand { get; }
        public ICommand GoBackCommand { get; }
        public ICommand ChangeFireModeCommand { get; }
        public ICommand QuitCommand { get; }

        private readonly string _udpHost = "192.168.2.189";
        private readonly int _udpPort = 9999;

        public AttackDisplayViewModel()
        {
            // 수동 발사 명령
            ManualFireCommand = new RelayCommand<object>(
                execute: async _ => SendManualFire(),
                canExecute: _ => true
            );

            // 비상폭파 명령
            EmergencyDestroyCommand = new RelayCommand<object>(
                 execute: _ => sendEmergencyDestroy(),
                 canExecute: _ => true
            );

            // 뒤로가기 버튼 초기화
            GoBackCommand = new RelayCommand<object>(
                 execute: _ => GoBack(),
                 canExecute: _ => NavigationService?.CanGoBack == true
            );

            // 발사 모드 변경 명령
            ChangeFireModeCommand = new RelayCommand<object>(
                execute: param => ChangeFireMode(param),
                canExecute: _ => true
            );

            // 종료 버튼 초기화
            QuitCommand = new RelayCommand<object>(
                 execute: _ => Quit(),
                 canExecute: _ => true
            );
        }

        // 서브시스템 정보
        private readonly List<(string url, string id)> subsystems = new()
        {
            ($"{Network.TCC}", "TCC"),
            ("http://192.168.2.66:8080", "TCC"),
            ($"{Network.MSS}", "MSS"),
            ($"{Network.ATS}", "ATS"),
            ($"{Network.LCH}", "LCH"),
            ($"{Network.MFR}", "MFR")
        };

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
                MessageBox.Show($"[OCC] 예외 발생: {ex.Message}");
                return false;
            }
        }

        private void ChangeFireMode(object? param)
        {
            if (param is string mode)
            {
                if (mode == "Auto")
                    FireMode = FireModeType.Auto;
                else if (mode == "Manual")
                    FireMode = FireModeType.Manual;

                // 패킷 전송 호출 추가
                SendFireModePacket();

                Debug.WriteLine($"발사 모드 변경: {FireMode}");
            }
        }

        /// <summary>
        /// 수동 발사 명령: 첫 번째 데이터 패킷 (명령코드:201)
        /// </summary>
        private void SendManualFire()
        {
            byte[] packet = CreateManualFirePacket();
            using var udpClient = new UdpClient();
            udpClient.Send(packet, packet.Length, _udpHost, _udpPort);
            Console.WriteLine("ManualFireCommand Dummy Packet 전송!");
        }

        /// <summary>
        /// 비상폭파 명령: 새 데이터 패킷 (명령코드:202, 비상폭파 패킷)
        /// </summary>
        private void sendEmergencyDestroy()
        {
            Debug.WriteLine("비상폭파 명령 전송 시작");
            byte[] packet = CreateEmergencyDestroyPacket();
            using var udpClient = new UdpClient();
            udpClient.Send(packet, packet.Length, _udpHost, _udpPort);
            Console.WriteLine("EmergencyDestroyCommand Dummy Packet 전송!");
        }

        /// <summary>
        /// 발사 모드 변경 패킷 생성 및 전송 (명령코드: 200)
        /// </summary>
        private void SendFireModePacket()
        {
            List<byte> packet = new List<byte>();

            // 1. 명령 코드 (200)
            packet.AddRange(BitConverter.GetBytes((UInt32)200));

            // 2. body 길이 (4)
            packet.AddRange(BitConverter.GetBytes((UInt32)4));

            // 3. 발사 모드 값 (0: 자동, 1: 수동)
            UInt32 modeValue = (FireMode == FireModeType.Auto) ? 0u : 1u;
            packet.AddRange(BitConverter.GetBytes(modeValue));

            // UDP 전송
            using var udpClient = new UdpClient();

            Debug.WriteLine($"{packet.ToString}");
            udpClient.Send(packet.ToArray(), packet.Count, _udpHost, _udpPort);

            Debug.WriteLine($"발사 모드 변경 패킷 전송 완료! (모드: {FireMode})");
        }

        /// <summary>
        /// 첫 번째 더미 패킷 생성: 명령코드 201, body 길이 28, 발사명령 식별자, 항공기 식별자
        /// </summary>
        private byte[] CreateManualFirePacket()
        {
            List<byte> packet = new List<byte>();

            // 1. 명령 코드 (201)
            packet.AddRange(BitConverter.GetBytes((UInt32)201));

            // 2. body 길이 (28)
            packet.AddRange(BitConverter.GetBytes((UInt32)28));

            // 3. 발사명령 식별자 (20Byte, 예: "MF-2024052812304500")
            string fireId = "MF-2024052812304500";
            byte[] fireIdBytes = Encoding.ASCII.GetBytes(fireId.PadRight(20, '\0'));
            packet.AddRange(fireIdBytes);

            // 4. 항공기 식별자 (8Byte, 예: "ATS-0000")
            string aircraftId = "ATS-0001";
            byte[] aircraftIdBytes = Encoding.ASCII.GetBytes(aircraftId.PadRight(8, '\0'));
            packet.AddRange(aircraftIdBytes);

            Debug.WriteLine($"수동 발사 패킷 전송 완료!");

            return packet.ToArray();
        }

        /// <summary>
        /// 비상폭파 명령용 새로운 더미 패킷 생성: 명령코드 202, body 길이 28, 비상폭파 명령 식별자, 미사일 식별자
        /// </summary>
        private byte[] CreateEmergencyDestroyPacket()
        {
            List<byte> packet = new List<byte>();

            // 1. 명령 코드 (202)
            packet.AddRange(BitConverter.GetBytes((UInt32)202));

            // 2. body 길이 (28)
            packet.AddRange(BitConverter.GetBytes((UInt32)28));

            // 3. 비상폭파 명령 식별자 (20Byte, 예: "ED-2024052812304500")
            string destroyId = "ED-2024052812304500";
            byte[] destroyIdBytes = Encoding.ASCII.GetBytes(destroyId.PadRight(20, '\0'));
            packet.AddRange(destroyIdBytes);

            // 4. 미사일 식별자 (8Byte, 예: "MSS-0000")
            string missileId = "MSS-0001";
            byte[] missileIdBytes = Encoding.ASCII.GetBytes(missileId.PadRight(8, '\0'));
            packet.AddRange(missileIdBytes);

            Debug.WriteLine($"비상 폭파 패킷 전송 완료!");

            return packet.ToArray();
        }
    }
}