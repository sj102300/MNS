using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http;
using System.Net.Sockets;
using System.Windows;
using System.Security.RightsManagement;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using OCC.Commands;
using OCC.Models;
using static OCC.ViewModels.AttackDisplayViewModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using OCC.Utils;
using System.Net;
using System.Windows.Navigation;
using Wpf.Ui;
using NavigationService = System.Windows.Navigation.NavigationService;
using System.Reflection;
using Newtonsoft.Json;
using System.Xml.Linq;
using static OCC.Models.FireMode;
using System.Reflection.PortableExecutable;
using OCC.Views;
using System.Windows.Controls;

namespace OCC.ViewModels
{
    public class AttackViewModel : BaseViewModel
    {
        private readonly FireMode _fireModeModel = new FireMode();
        public FireMode.FireModeType FireMode
        {
            get => _fireModeModel.Mode;
            set
            {
                if (_fireModeModel.Mode != value)
                {
                    _fireModeModel.Mode = value;
                    Debug.WriteLine($"[ViewModel] FireMode 변경됨: {value}");
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(IsAutoFireMode));
                    OnPropertyChanged(nameof(IsManualFireMode));
                    OnPropertyChanged(nameof(ChangeModeText));
                }
            }
        }
        // AircraftList 프로퍼티 추가
        public AircraftList AircraftList { get; } = new AircraftList();

        private AircraftWithIp _selectedAircraft;
        public AircraftWithIp SelectedAircraft
        {
            get => _selectedAircraft;
            set
            {
                if (_selectedAircraft != value)
                {
                    _selectedAircraft = value;
                    // 선택된 항공기 식별자 저장
                    OnPropertyChanged();
                }
            }
        }

        public bool IsAutoFireMode => FireMode == Models.FireMode.FireModeType.Auto;
        public bool IsManualFireMode => FireMode == Models.FireMode.FireModeType.Manual;
        public string ChangeModeText => FireMode == FireModeType.Auto ? "자동 발사 모드 OFF" : "자동 발사 모드 ON";

        private static string commandId = "MF-2024052812304500"; // 예시 발사 명령 식별자

        public ICommand QuitCommand { get; }
        public ICommand ChangeModeCommand { get; }
        public ICommand ManualFireCommand { get; }
        public ICommand EmergencyDestroyCommand { get; }
        //public ICommand SelectAircraftCommand { get; }

        public AttackViewModel(NavigationService navigationService)
        {
            NavigationService = navigationService ?? throw new ArgumentNullException(nameof(navigationService));

            QuitCommand = new RelayCommand<object>(
                 execute: _ => Quit(),
                 canExecute: _ => true
            );

            ManualFireCommand = new RelayCommand<object>(
                execute: _ => SendManualFirePacketAsync(commandId, SelectedAircraft),
                canExecute: _ => true // 필요시 조건 추가
            );

            ChangeModeCommand = new RelayCommand<object>(
                execute: _ => SendFireModePacketAsync(),
                canExecute: _ => true // 필요시 조건 추가
            );

            EmergencyDestroyCommand = new RelayCommand<object>(
                execute: _ => SendEmergencyDestroyPacketAsync(SelectedMissile),
                canExecute: _ => true
            );

            //SelectAircraftCommand = new RelayCommand<object>(
            //    execute: param => selectAircraft(param),
            //    canExecute: _ => true
            //);

            StartReceiving();
        }

        //public event PropertyChangedEventHandler PropertyChanged;

        //protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        //    => OnPropertyChanged(propertyName);

        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            Debug.WriteLine($"[DEBUG] OnPropertyChanged 호출됨: {propertyName}");
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        // 기존 코드 유지
        private void SendFireModePacketAsync()
        {
            Task.Run(async () =>
            {
                const int CommandCode = 200;
                const int BodyLength = 4;
                const int RetryCount = 10;
                const int RetryIntervalMs = 100;

                IPEndPoint remoteEP = new IPEndPoint(IPAddress.Parse(Network.TCCHost), Network.TCCHostPort);
                IPEndPoint localEP = new IPEndPoint(IPAddress.Any, 0);

                using var udpClient = new UdpClient();
                udpClient.Client.ReceiveTimeout = RetryIntervalMs;

                // 1. 패킷 구성
                List<byte> packet = new List<byte>();
                packet.AddRange(BitConverter.GetBytes((UInt32)CommandCode));
                packet.AddRange(BitConverter.GetBytes((UInt32)BodyLength));
                UInt32 modeValue = (FireMode == Models.FireMode.FireModeType.Auto) ? 0u : 1u;
                packet.AddRange(BitConverter.GetBytes(modeValue));
                byte[] buffer = packet.ToArray();

                bool ackReceived = false;

                for (int i = 0; i < RetryCount; i++)
                {
                    try
                    {
                        // 2. 송신
                        udpClient.Send(buffer, buffer.Length, remoteEP);
                        Debug.WriteLine($"[{i + 1}] 발사 모드 변경 패킷 전송! (모드: {FireMode})");

                        // 3. 수신 대기
                        var response = udpClient.Receive(ref localEP);
                        if (response != null && response.Length > 0)
                        {
                            ackReceived = true;
                            Debug.WriteLine("ACK 수신 완료!");
                            break;
                        }
                    }
                    catch (SocketException ex)
                    {
                        Debug.WriteLine($"[{i + 1}] ACK 수신 실패: {ex.Message}");
                    }

                    await Task.Delay(RetryIntervalMs);
                }

                // 4. ACK 수신 시 모드 전환
                if (ackReceived)
                {
                    Application.Current.Dispatcher.Invoke(() =>
                    {
                        FireMode = (FireMode == Models.FireMode.FireModeType.Auto)
                            ? Models.FireMode.FireModeType.Manual
                            : Models.FireMode.FireModeType.Auto;


#if true
                        Debug.WriteLine(FireMode);
                        MessageBox.Show($"ACK 수신 → 모드 전환 완료: {FireMode}");
#endif 
                    });
                }
                else
                {
                    MessageBox.Show("ACK 미수신 → 모드 유지");
                }
            });
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

        private void SendManualFirePacketAsync(string commandId, AircraftWithIp aircraft)
        {
            if(aircraft == null)
            {
                MessageBox.Show("선택된 항공기가 없습니다!");
                return;
            }
            Task.Run(async () =>
            {
                const int CommandCode = 201;
                const int BodyLength = 28;
                const int RetryCount = 10;
                const int RetryIntervalMs = 100;

                IPEndPoint remoteEP = new IPEndPoint(IPAddress.Parse(Network.TCCHost), Network.TCCHostPort);
                IPEndPoint localEP = new IPEndPoint(IPAddress.Any, 0);

                using var udpClient = new UdpClient();
                udpClient.Client.ReceiveTimeout = RetryIntervalMs;

                // 문자열 바이트 변환 (고정 크기로 padding 포함)
                byte[] commandIdBytes = new byte[20];
                byte[] srcCommandId = Encoding.ASCII.GetBytes(commandId);
                Array.Copy(srcCommandId, commandIdBytes, Math.Min(srcCommandId.Length, 20));

                byte[] aircraftIdBytes = new byte[8];
                byte[] srcAircraftId = Encoding.ASCII.GetBytes(aircraft.AircraftId);
                Array.Copy(srcAircraftId, aircraftIdBytes, Math.Min(srcAircraftId.Length, 8));

                // 패킷 생성
                List<byte> packet = new List<byte>();
                packet.AddRange(BitConverter.GetBytes((UInt32)CommandCode));
                packet.AddRange(BitConverter.GetBytes((Int32)BodyLength));
                packet.AddRange(commandIdBytes);
                packet.AddRange(aircraftIdBytes);

                byte[] buffer = packet.ToArray();
                bool ackReceived = false;

                for (int i = 0; i < RetryCount; i++)
                {
                    try
                    {
                        udpClient.Send(buffer, buffer.Length, remoteEP);
                        Debug.WriteLine($"[{i + 1}] 발사 명령 전송! (commandId={commandId}, aircraftId={aircraft.AircraftId})");

                        var response = udpClient.Receive(ref localEP);
                        if (response != null && response.Length > 0)
                        {
                            ackReceived = true;
                            Debug.WriteLine("발사 ACK 수신 완료!");
                            break;
                        }
                    }
                    catch (SocketException ex)
                    {
                        Debug.WriteLine($"[{i + 1}] 발사 ACK 수신 실패: {ex.Message}");
                    }

                    await Task.Delay(RetryIntervalMs);
                }

                if (!ackReceived)
                {
                    Debug.WriteLine("발사 ACK 미수신 → 재시도 종료, 실패 처리 필요");
                }
            });
        }


        /// <summary>
        /// 비상폭파 명령용 새로운 더미 패킷 생성: 명령코드 202, body 길이 28, 비상폭파 명령 식별자, 미사일 식별자
        /// </summary>
        private byte[] CreateEmergencyDestroyPacket(string missileId)
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

            // 4. 미사일 식별자 (8Byte, 예: "MSS-0000")missileId
            byte[] missileIdBytes = Encoding.ASCII.GetBytes(missileId.PadRight(8, '\0'));
            packet.AddRange(missileIdBytes);

            Debug.WriteLine($"비상 폭파 패킷 전송 완료!");

            return packet.ToArray();
        }
        /// <summary>
        /// 비상폭파 명령: 새 데이터 패킷 (명령코드:202, 비상폭파 패킷)
        /// </summary>
        //private void sendEmergencyDestroy()
        //{
        //    Debug.WriteLine("비상폭파 명령 전송 시작");
        //    byte[] packet = CreateEmergencyDestroyPacket();
        //    using var udpClient = new UdpClient();
        //    udpClient.Send(packet, packet.Length, Network.TCCHost, Network.TCCHostPort);
        //    Console.WriteLine("EmergencyDestroyCommand Dummy Packet 전송!");
        //}
        public void StartReceiving()
        {
            UdpReceiver.Start(AircraftList);
        }

        private readonly List<(string url, string id)> subsystems = new()
        {
            ($"http://192.168.2.66:8080", "TCC"),
            ($"{Network.ATS}", "ATS"),
            ($"{Network.MFR}", "MFR")
            //($"{Network.MSS}", "MSS"),
            //($"{Network.LCH}", "LCH"),
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
                    //InitPage로 네비게이션
                    if (NavigationService != null)
                    {
                        var mainWindow = Application.Current.Windows.OfType<MainWindow>().FirstOrDefault();
                        if (mainWindow != null)
                        {
                            var frame = mainWindow.FindName("MainFrame") as Frame;
                            if (frame != null)
                            {
                                var initPage = new OCC.Views.InitPage(NavigationService);
                                frame.Navigate(initPage);
                            }
                        }
                    }
                    else
                    {
                        Debug.WriteLine("NavigationService가 설정되지 않았습니다.");
                    }

                    var currentWindow = Application.Current.Windows.OfType<Window>().FirstOrDefault(w => w.IsActive);
                    foreach (var win in Application.Current.Windows.OfType<Window>().ToList())
                    {
                        if (win != currentWindow)
                        {
                            win.Close();
                        }
                    }
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
                MessageBox.Show($"[OCC] {subsystemId}예외 발생: {ex.Message}");
                return false;
            }
        }
        public MissileList MissileList { get; } = new MissileList();

        public Missile _selectedMissile { get; set; }
        public Missile SelectedMissile
        {
            get => _selectedMissile;
            set
            {
                if (_selectedMissile != value)
                {
                    _selectedMissile = value;
                    OnPropertyChanged();
                }
            }
        }

        private void SendEmergencyDestroyPacketAsync(Missile missile)
        {
            if(missile == null)
            {
                MessageBox.Show("미사일을 선택하세요!");
                return;
            }
            Task.Run(async () =>
            {
                const int CommandCode = 200;
                const int BodyLength = 4;
                const int RetryCount = 10;
                const int RetryIntervalMs = 100;

                IPEndPoint remoteEP = new IPEndPoint(IPAddress.Parse(Network.TCCHost), Network.TCCHostPort);
                IPEndPoint localEP = new IPEndPoint(IPAddress.Any, 0);

                using var udpClient = new UdpClient();
                udpClient.Client.ReceiveTimeout = RetryIntervalMs;

                string missileId = missile != null ? missile.MissileId  : "MSS-100";
                byte[] buffer = CreateEmergencyDestroyPacket(missileId);

                bool ackReceived = false;

                for (int i = 0; i < RetryCount; i++)
                {
                    try
                    {
                        // 2. 송신
                        udpClient.Send(buffer, buffer.Length, remoteEP);
                        Debug.WriteLine($"[{i + 1}] 발사 모드 변경 패킷 전송! (모드: {FireMode})");

                        // 3. 수신 대기
                        var response = udpClient.Receive(ref localEP);
                        if (response != null && response.Length > 0)
                        {
                            ackReceived = true;
                            Debug.WriteLine("ACK 수신 완료!");
                            break;
                        }
                    }
                    catch (SocketException ex)
                    {
                        Debug.WriteLine($"[{i + 1}] ACK 수신 실패: {ex.Message}");
                    }

                    await Task.Delay(RetryIntervalMs);
                }

                // 4. ACK 수신 시 모드 전환
                if (ackReceived)
                {
                    Application.Current.Dispatcher.Invoke(() =>
                    {
                        //할거없나?
#if true
                        Debug.WriteLine(FireMode);
                        MessageBox.Show($"ACK 수신 → 비상 폭파 명령 송신 완료: {missileId}");
#endif 
                    });
                }
                else
                {
                    MessageBox.Show($"ACK 미수신 → 비상 폭파 명령 송신 실패: {missileId}");
                }
            });
        }
    }
}
