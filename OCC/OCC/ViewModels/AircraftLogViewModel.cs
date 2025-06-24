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
//using static OCC.ViewModels.AttackDisplayViewModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using OCC.Utils;
using System.Net;
using System.Windows.Navigation;
using Wpf.Ui;
using NavigationService = System.Windows.Navigation.NavigationService;

namespace OCC.ViewModels
{
    internal class AircraftLogViewModel
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
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(IsAutoFireMode));
                    OnPropertyChanged(nameof(IsManualFireMode));
                    OnPropertyChanged(nameof(ChangeModeText));
                }
            }
        }

        public bool IsAutoFireMode => FireMode == Models.FireMode.FireModeType.Auto;
        public bool IsManualFireMode => FireMode == Models.FireMode.FireModeType.Manual;

        public string ChangeModeText => FireMode == Models.FireMode.FireModeType.Auto ? "자동 발사 모드 OFF" : "자동 발사 모드 ON";

        private static string commandId = "MF-2024052812304500"; // 예시 발사 명령 식별자
        private static string selectedAircraftId = "ATS-0001"; // 예시 항공기 식별자

        public ICommand ChangeModeCommand { get; }
        public ICommand ManualFireCommand { get; }


        private readonly NavigationService _navigationService;

        public AircraftLogViewModel()
        {
            ManualFireCommand = new RelayCommand<object>(
                execute: _ => SendManualFirePacketAsync(commandId, selectedAircraftId),
                canExecute: _ => true // 필요시 조건 추가
            );

            ChangeModeCommand = new RelayCommand<object>(
                execute: _ => SendFireModePacketAsync(),
                canExecute: _ => true // 필요시 조건 추가
            );
        }

        public AircraftLogViewModel(NavigationService navigationService)
        {
            _navigationService = navigationService ?? throw new ArgumentNullException(nameof(navigationService));

            ManualFireCommand = new RelayCommand<object>(
                execute: _ => SendManualFirePacketAsync(commandId, selectedAircraftId),
                canExecute: _ => true // 필요시 조건 추가
            );

            ChangeModeCommand = new RelayCommand<object>(
                execute: _ => SendFireModePacketAsync(),
                canExecute: _ => true // 필요시 조건 추가
            );
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
            => OnPropertyChanged(propertyName);

        //protected void OnPropertyChanged(string propertyName)
        //    => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));

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
                    await Application.Current.Dispatcher.BeginInvoke(() =>
                    {
                        FireMode = (FireMode == Models.FireMode.FireModeType.Auto)
                            ? Models.FireMode.FireModeType.Manual
                            : Models.FireMode.FireModeType.Auto;

                        Debug.WriteLine($"ACK 수신 → 모드 전환 완료: {FireMode}");
                    });
                }
                else
                {
                    Debug.WriteLine("ACK 미수신 → 모드 유지");
                }
            });
        }

        private void SendManualFirePacketAsync(string commandId, string aircraftId)
        {
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
                byte[] srcAircraftId = Encoding.ASCII.GetBytes(aircraftId);
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
                        Debug.WriteLine($"[{i + 1}] 발사 명령 전송! (commandId={commandId}, aircraftId={aircraftId})");

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
        /// 발사 모드 변경 패킷 생성 및 전송 (명령코드: 200)
        /// </summary>
        //private void SendFireModePacket()
        //{
        //    List<byte> packet = new List<byte>();

        //    // 1. 명령 코드 (200)
        //    packet.AddRange(BitConverter.GetBytes((UInt32)200));

        //    // 2. body 길이 (4)
        //    packet.AddRange(BitConverter.GetBytes((UInt32)4));

        //    // 3. 발사 모드 값 (0: 자동, 1: 수동)
        //    UInt32 modeValue = (FireMode == Models.FireMode.FireModeType.Auto) ? 0u : 1u;
        //    packet.AddRange(BitConverter.GetBytes(modeValue));

        //    // UDP 전송
        //    using var udpClient = new UdpClient();

        //    Debug.WriteLine($"{packet.ToString}");
        //    udpClient.Send(packet.ToArray(), packet.Count, Network.TCCHost, Network.TCCHostPort);

        //    Debug.WriteLine($"발사 모드 변경 패킷 전송 완료! (모드: {FireMode})");
        //}


        //private void ChangeFireMode(object? param)
        //{
        //    if (param is string mode)
        //    {
        //        if (mode == "Auto")
        //            FireMode = FireModeType.Auto;
        //        else if (mode == "Manual")
        //            FireMode = FireModeType.Manual;

        //        // 패킷 전송 호출 추가
        //        SendFireModePacket();

        //        Debug.WriteLine($"발사 모드 변경: {FireMode}");
        //    }
        //}

        /// <summary>
        /// 수동 발사 명령: 첫 번째 데이터 패킷 (명령코드:201)
        /// </summary>
        //private void SendManualFire()
        //{
        //    byte[] packet = CreateManualFirePacket();
        //    using var udpClient = new UdpClient();
        //    udpClient.Send(packet, packet.Length, _udpHost, _udpPort);
        //    Console.WriteLine("ManualFireCommand Dummy Packet 전송!");
        //}

        ///// <summary>
        ///// 비상폭파 명령: 새 데이터 패킷 (명령코드:202, 비상폭파 패킷)
        ///// </summary>
        //private void sendEmergencyDestroy()
        //{
        //    Debug.WriteLine("비상폭파 명령 전송 시작");
        //    byte[] packet = CreateEmergencyDestroyPacket();
        //    using var udpClient = new UdpClient();
        //    udpClient.Send(packet, packet.Length, _udpHost, _udpPort);
        //    Console.WriteLine("EmergencyDestroyCommand Dummy Packet 전송!");
        //}

        ///// <summary>
        ///// 첫 번째 더미 패킷 생성: 명령코드 201, body 길이 28, 발사명령 식별자, 항공기 식별자
        ///// </summary>
        //private byte[] CreateManualFirePacket()
        //{
        //    List<byte> packet = new List<byte>();

        //    // 1. 명령 코드 (201)
        //    packet.AddRange(BitConverter.GetBytes((UInt32)201));

        //    // 2. body 길이 (28)
        //    packet.AddRange(BitConverter.GetBytes((UInt32)28));

        //    // 3. 발사명령 식별자 (20Byte, 예: "MF-2024052812304500")
        //    string fireId = "MF-2024052812304500";
        //    byte[] fireIdBytes = Encoding.ASCII.GetBytes(fireId.PadRight(20, '\0'));
        //    packet.AddRange(fireIdBytes);

        //    // 4. 항공기 식별자 (8Byte, 예: "ATS-0000")
        //    string aircraftId = "ATS-0001";
        //    byte[] aircraftIdBytes = Encoding.ASCII.GetBytes(aircraftId.PadRight(8, '\0'));
        //    packet.AddRange(aircraftIdBytes);

        //    Debug.WriteLine($"수동 발사 패킷 전송 완료!");

        //    return packet.ToArray();
        //}

        ///// <summary>
        ///// 비상폭파 명령용 새로운 더미 패킷 생성: 명령코드 202, body 길이 28, 비상폭파 명령 식별자, 미사일 식별자
        ///// </summary>
        //private byte[] CreateEmergencyDestroyPacket()
        //{
        //    List<byte> packet = new List<byte>();

        //    // 1. 명령 코드 (202)
        //    packet.AddRange(BitConverter.GetBytes((UInt32)202));

        //    // 2. body 길이 (28)
        //    packet.AddRange(BitConverter.GetBytes((UInt32)28));

        //    // 3. 비상폭파 명령 식별자 (20Byte, 예: "ED-2024052812304500")
        //    string destroyId = "ED-2024052812304500";
        //    byte[] destroyIdBytes = Encoding.ASCII.GetBytes(destroyId.PadRight(20, '\0'));
        //    packet.AddRange(destroyIdBytes);

        //    // 4. 미사일 식별자 (8Byte, 예: "MSS-0000")
        //    string missileId = "MSS-0001";
        //    byte[] missileIdBytes = Encoding.ASCII.GetBytes(missileId.PadRight(8, '\0'));
        //    packet.AddRange(missileIdBytes);

        //    Debug.WriteLine($"비상 폭파 패킷 전송 완료!");

        //    return packet.ToArray();
        //}

    }
}
