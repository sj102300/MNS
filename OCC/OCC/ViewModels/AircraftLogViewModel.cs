using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http;
using System.Net.Sockets;
using System.Security.RightsManagement;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using OCC.Commands;
using OCC.Models;
using static OCC.ViewModels.AttackDisplayViewModel;

namespace OCC.ViewModels
{
    internal class AircraftLogViewModel
    {
        public ICommand ManualFireCommand { get; }
        public ICommand ChangeModeCommand { get; }

        public AircraftLogViewModel()
        {
            ManualFireCommand = new RelayCommand<object>(
                execute: _ => ExecuteManualFire(),
                canExecute: _ => true // 필요시 조건 추가
            );

            //ChangeModeCommand = new RelayCommand<object>(
            //    execute: param => ChangeFireMode(param),
            //    canExecute: _ => true // 필요시 조건 추가
            //);
        }

        private void ExecuteManualFire()
        {
            // 수동 발사 로직을 여기에 작성하세요.
            
        }

        ///// <summary>
        ///// 발사 모드 변경 패킷 생성 및 전송 (명령코드: 200)
        ///// </summary>
        //private void SendFireModePacket()
        //{
        //    List<byte> packet = new List<byte>();

        //    // 1. 명령 코드 (200)
        //    packet.AddRange(BitConverter.GetBytes((UInt32)200));

        //    // 2. body 길이 (4)
        //    packet.AddRange(BitConverter.GetBytes((UInt32)4));

        //    // 3. 발사 모드 값 (0: 자동, 1: 수동)
        //    //UInt32 modeValue = (FireModeType == FireModeType.Auto) ? 0u : 1u;
        //    //packet.AddRange(BitConverter.GetBytes(modeValue));

        //    // UDP 전송
        //    using var udpClient = new UdpClient();

        //    Debug.WriteLine($"{packet.ToString}");
        //    udpClient.Send(packet.ToArray(), packet.Count, _udpHost, _udpPort);

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

        ///// <summary>
        ///// 수동 발사 명령: 첫 번째 데이터 패킷 (명령코드:201)
        ///// </summary>
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
