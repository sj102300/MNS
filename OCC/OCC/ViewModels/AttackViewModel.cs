﻿using System;
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
using System.Windows.Media;
using OCC.Views;
using System.Windows.Controls;
using GMap.NET;
using System.Collections.ObjectModel;
using GMap.NET.WindowsPresentation;
using System.Windows.Shapes;
using System.ComponentModel.Design;
using System.Windows.Media.Imaging;
using WpfAnimatedGif;


namespace OCC.ViewModels
{
    public class AttackViewModel : BaseViewModel
    {
        private FireModeType _fireMode = FireModeType.Auto;

        public FireMode.FireModeType FireMode
        {
            get => _fireMode;
            set
            {
                if (SetProperty(ref _fireMode, value))
                {
                    Debug.WriteLine($"[ViewModel] FireMode 변경됨: {value}");
                    Debug.WriteLine($"[ViewModel] ChangeModeText → {ChangeModeText}");
                    OnPropertyChanged(nameof(IsAutoFireMode));
                    OnPropertyChanged(nameof(IsManualFireMode));
                    OnPropertyChanged(nameof(ChangeModeText));
                    OnPropertyChanged(nameof(ChangeModeButtonBackground));
                }
            }
        }
        // AircraftList 프로퍼티 추가

        public ObservableCollection<AircraftWithIp> AircraftList { get; } = new();
        private readonly Dictionary<string, AircraftWithIp> aircraftLookup = new();  // 빠른 검색을 위해
        public ObservableCollection<ImpactPoint> ImpactPointList { get; } = new();
        private readonly Dictionary<string, ImpactPoint> impactPointLookup = new();  // 빠른 검색을 위해
        public ObservableCollection<Missile> MissileList { get; } = new();
        private readonly Dictionary<string, Missile> missileLookup = new(); // 빠른 검색을 위해


        public bool IsAutoFireMode => FireMode == Models.FireMode.FireModeType.Auto;
        public bool IsManualFireMode => FireMode == Models.FireMode.FireModeType.Manual;
        public string ChangeModeText => FireMode == FireModeType.Auto ? "자동 발사 모드 OFF" : "자동 발사 모드 ON";
        public Brush ChangeModeButtonBackground => FireMode == FireModeType.Auto ? Brushes.Green : Brushes.LightGray;

        //private static string commandId = "MF-2024052812304500"; // 예시 발사 명령 식별자

        public ICommand QuitCommand { get; }
        public ICommand ChangeModeCommand { get; }
        public ICommand ManualFireCommand { get; }
        public ICommand EmergencyDestroyCommand { get; }
        //public ICommand SelectAircraftCommand { get; }

        public AttackViewModel(NavigationService navigationService)
        {
            //Debug.WriteLine($"[AttackViewModel 생성됨] HashCode: {this.GetHashCode()}");

            NavigationService = navigationService ?? throw new ArgumentNullException(nameof(navigationService));

            QuitCommand = new RelayCommand<object>(
                 execute: _ => Quit(),
                 canExecute: _ => true
            );

            ManualFireCommand = new RelayCommand<object>(
                execute: _ => SendManualFirePacketAsync(SelectedAircraft),
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

            StartReceiving();
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
                UInt32 modeValue = (FireMode == Models.FireMode.FireModeType.Auto) ? 1u : 0u;
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
        public async Task<Coordinate?> GetScenarioInfoAsync(string scenarioId)
        {
            try
            {
                using var client = new HttpClient();
                client.Timeout = TimeSpan.FromSeconds(3);
                // 실제 SCN API 주소와 파라미터로 교체
                string url = $"{Network.SCN}/scenario/info"; // 예시
                
                // POST body 생성
                var postData = new
                {
                    scenario_id = scenarioId
                };
                var jsonBody = JsonConvert.SerializeObject(postData);
                var content = new StringContent(jsonBody, Encoding.UTF8, "application/json");
                
                var response = await client.PostAsync(url, content);
                if (!response.IsSuccessStatusCode)
                    return null;

                string json = await response.Content.ReadAsStringAsync();

                // 예시: Coordinate 객체로 역직렬화
                var coord = JsonConvert.DeserializeObject<ScenarioInfo>(json);
                //Debug.WriteLine($"-----------------------{coord.coordinate.latitude}------{coord.coordinate.longitude}---------------------");
                return coord.coordinate;
            }
            catch
            {
                return null;
            }
        }
        private static string GenerateLaunchCommandId()
        {
            // 현재 날짜 및 시간 가져오기
            DateTime now = DateTime.Now;

            // "MF-" + yyyyMMddHHmmssfff 형식으로 포맷
            return "MF-" + now.ToString("yyyyMMddHHmmssfff");
        }

        private void SendManualFirePacketAsync(AircraftWithIp aircraft)
        {
            if(aircraft == null)
            {
                MessageBox.Show("선택된 항공기가 없습니다!");
                return;
            }
            Task.Run(async () =>
            {
                string commandId = GenerateLaunchCommandId();
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
                byte[] srcAircraftId = Encoding.ASCII.GetBytes(aircraft.Id);
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
                        Debug.WriteLine($"[{i + 1}] 발사 명령 전송! (commandId={commandId}, aircraftId={aircraft.Id})");

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

        public void StartReceiving()
        {
            UdpReceiver.AircraftReceived += OnAircraftReceived;
            UdpReceiver.MissileReceived += OnMissileReceived;
            UdpReceiver.ImpactPointReceived += OnImpactPointReceived;
            UdpReceiver.Start(AircraftList, aircraftLookup, MissileList, missileLookup, ImpactPointList, impactPointLookup);
        }

        private void OnMissileReceived(string id, double lat, double lon, double alt, uint status)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                if (missileLookup.TryGetValue(id, out var missile))
                {
                    missile.Latitude = lat;
                    missile.Longitude = lon;
                    missile.Altitude = alt;
                    missile.Status = status;
                }
                else
                {
                    // 새로운 미사일 정보가 들어온 경우
                    var newMissile = new Missile(id)
                    {
                        Latitude = lat,
                        Longitude = lon,
                        Altitude = alt,
                        Status = status
                    };
                    MissileList.Add(newMissile);
                    missileLookup[id] = newMissile;

                    // missile_id 기준으로 MissileList 정렬
                    var sorted = MissileList.OrderBy(m => m.Id).ToList();
                    MissileList.Clear();
                    foreach (var m in sorted)
                        MissileList.Add(m);
                }
            });
        }
        private void OnImpactPointReceived(string commandId, string aircraftId, string missileId, double lat, double lon, double alt)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                if (impactPointLookup.TryGetValue(commandId, out var ip))
                {
                    ip.Latitude = lat;
                    ip.Longitude = lon;
                    ip.Altitude = alt;
                }
                else
                {
                    // 새로운 정보가 들어온 경우
                    var newIP = new ImpactPoint(commandId)
                    {
                        AircraftId = aircraftId,
                        MissileId = missileId,
                        Latitude = lat,
                        Longitude = lon,
                        Altitude = alt,
                    };
                    ImpactPointList.Add(newIP);
                    impactPointLookup[commandId] = newIP;
                }
            });
        }

        private void OnAircraftReceived(string id, double lat, double lon, double alt, uint status, uint foe, double iplat, double iplon, double ipalti)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                if (aircraftLookup.TryGetValue(id, out var ac))
                {
                    ac.Latitude = lat;
                    ac.Longitude = lon;
                    ac.Altitude = alt;
                    ac.Status = status;
                    ac.Foe = foe;
                    ac.IpLatitude = iplat;
                    ac.IpLongitude = iplon;
                    ac.IpAltitude = ipalti;
                }
                else
                {
                    // 새로운 항공기 정보가 들어온 경우
                    var newAc = new AircraftWithIp(id)
                    {
                        Latitude = lat,
                        Longitude = lon,
                        Altitude = alt,
                        Status = status,
                        Foe = foe,
                        IpLatitude = iplat,
                        IpLongitude = iplon,
                        IpAltitude = ipalti
                    };
                    AircraftList.Add(newAc);
                    aircraftLookup[id] = newAc;
                }
            });
        }


        public void QuitReceiving()
        {
            UdpReceiver.Stop();
        }

        private readonly List<(string url, string id)> subsystems = new()
        {
            ($"{Network.TCC}", "TCC"),
            ($"{Network.ATS}", "ATS"),
            ($"{Network.MFR}", "MFR"),
            ($"{Network.MSS}", "MSS"),
            ($"{Network.LCH}", "LCH"),
        };
        private async void Quit()
        {
            // 소켓 닫기
            OCC.Utils.UdpReceiver.Stop();

            Debug.WriteLine("종료 요청");
            // 모든 서브시스템에 대해 비동기 요청을 병렬로 실행
            QuitReceiving();
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
                //MessageBox.Show($"[OCC] {subsystemId}예외 발생: {ex.Message}");
                return false;
            }
        }
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

        public AircraftWithIp _selectedAircraft { get; set; }

        public AircraftWithIp SelectedAircraft
        {
            get => _selectedAircraft;
            set
            {
                if (_selectedAircraft != value)
                {
                    _selectedAircraft = value;
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

                string missileId = missile != null ? missile.Id  : "MSS-100";
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
                        Debug.WriteLine(FireMode);
                        //MessageBox.Show($"ACK 수신 → 비상 폭파 명령 송신 완료: {missileId}");
                    });
                }
                else
                {
                    MessageBox.Show($"ACK 미수신 → 비상 폭파 명령 송신 실패: {missileId}");
                }
            });
        }

        // ㅇㅅㅇ
        private PointLatLng? _batteryPos;      // null ⇒ 아직 안 받은 상태
        public PointLatLng? BatteryPos
        {
            get => _batteryPos;
            private set
            {
                if (_batteryPos != value)
                {
                    _batteryPos = value;
                    OnPropertyChanged();        // View 에 통보
                }
            }
        }

        /* 외부(시나리오)에서 한 번 호출 */
        public void SetBatteryPos(double lat, double lon)
        {
            BatteryPos = new PointLatLng(lat, lon);
        }

        //private void LoadGif(string relativePath)
        //{
        //    try
        //    {
        //        var uri = new Uri($"pack://application:,,,/{relativePath}", UriKind.Absolute);
        //        var image = new BitmapImage(uri);
        //        ImageBehavior.SetAnimatedSource(MissileGifImage, image);
        //    }
        //    catch (Exception ex)
        //    {
        //        Debug.WriteLine($"[GIF 로딩 실패] {ex.Message}");
        //    }
        //}

        //public GMapMarker CreateMarker(PointLatLng pos, Brush fill, double size)
        //{
        //    return new GMapMarker(pos)
        //    {
        //        Shape = new Ellipse
        //        {
        //            Width = size,
        //            Height = size,
        //            Fill = fill,
        //            Stroke = Brushes.Black,
        //            StrokeThickness = 1.5,
        //            Opacity = 0.9
        //        },
        //        Offset = new Point(-size / 2, -size / 2)
        //    };
        //}

    }
}
