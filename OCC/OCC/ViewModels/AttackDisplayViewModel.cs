using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using OCC.Commands;

namespace OCC.ViewModels
{
    public class AttackDisplayViewModel : BaseViewModel
    {
        // 발사 모드 enum
        public enum FireModeType { Auto, Manual }

        private FireModeType _fireMode = FireModeType.Manual;
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
        }

        private void ChangeFireMode(object? param)
        {
            if (param is string mode)
            {
                if (mode == "Auto")
                    FireMode = FireModeType.Auto;
                else if (mode == "Manual")
                    FireMode = FireModeType.Manual;
                // 필요시 모드 변경시 추가 동작 구현
                Console.WriteLine($"발사 모드 변경: {FireMode}");
            }
        }

        private readonly string _udpHost = "127.0.0.1";
        private readonly int _udpPort = 12345;

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
            byte[] packet = CreateEmergencyDestroyPacket();
            using var udpClient = new UdpClient();
            udpClient.Send(packet, packet.Length, _udpHost, _udpPort);
            Console.WriteLine("EmergencyDestroyCommand Dummy Packet 전송!");
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
            string aircraftId = "ATS-0000";
            byte[] aircraftIdBytes = Encoding.ASCII.GetBytes(aircraftId.PadRight(8, '\0'));
            packet.AddRange(aircraftIdBytes);

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

            return packet.ToArray();
        }
    }
}