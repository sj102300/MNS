using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using OCC.Models;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Diagnostics;
using System.Windows.Documents;
using System.Windows;
using System.Collections.ObjectModel;

namespace OCC.Utils
{
    public static class UdpReceiver
    {
        private static UdpClient? udp;
        private static CancellationTokenSource? cts;

        public static event Action<string, double, double, double, uint, uint, double, double, double> AircraftReceived;
        public static event Action<string, string, string, double,double, double> ImpactPointReceived;
        public static event Action<string, double, double, double, uint> MissileReceived;
        public static event Action<string, string, string, uint> MissileDestroyReceived;

        public static void Start(ObservableCollection<AircraftWithIp> aircraftList, Dictionary<string, AircraftWithIp> aircraftLookup,
            ObservableCollection<Missile> missileList, Dictionary<string, Missile> missileLookup,
            ObservableCollection<ImpactPoint> impactPointList, Dictionary<string, ImpactPoint> impactPointLookup)
        {
            cts = new CancellationTokenSource();
            var token = cts.Token;

            Task.Run(() =>
            {
                IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.99"), 9001);        //승엽
                //IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.194"), 9001);       //승주
                //IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.195"), 9001);       //명준
                //IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.200"), 9999);
                using var udp = new UdpClient(ep);

                Debug.WriteLine("항공기 정보 수신 시작");

                try
                {
                    while (!token.IsCancellationRequested)
                    {
                        if (udp.Available > 0)
                        {
                            var remote = new IPEndPoint(IPAddress.Any, 0);
                            byte[] data = udp.Receive(ref remote);

                            uint cmd = BitConverter.ToUInt32(data, 0);
                            if (cmd == 100)
                            {
                                ParseAircraft(data.Skip(8).ToArray());
                            }
                            else if(cmd == 201)
                            {
                                ParseLaunchCommand(data.Skip(8).ToArray());
                            }
                            else if(cmd == 203)
                            {
                                ParseMissileDestroy(data.Skip(8).ToArray());
                            }
                            else if (cmd == 300)
                            {
                                ParseMissile(data.Skip(8).ToArray());
                            }
                        }
                        else
                        {
                            Thread.Sleep(10);
                        }
                    }
                }
                catch (ObjectDisposedException)
                {
                    // 소켓이 닫힐 때 예외 무시
                }
                finally
                {
                    udp?.Close();
                    udp?.Dispose();
                }
            }, token);
        }

        public static void Stop()
        {
            cts?.Cancel();
            udp?.Close();
            udp?.Dispose();
            udp = null;
            cts?.Dispose();
            cts = null;
            Debug.WriteLine("UdpReceiver 중지됨");
        }

        private static void ParseAircraft(byte[] body)
        {
            string id = Encoding.ASCII.GetString(body, 0, 8).Trim('\0');
            double lat = BitConverter.ToDouble(body, 8);
            double lon = BitConverter.ToDouble(body, 16);
            double alt = BitConverter.ToDouble(body, 24);
            uint status = BitConverter.ToUInt32(body, 36);
            uint foe = BitConverter.ToUInt32(body, 32);
            double ipLat = BitConverter.ToDouble(body, 40);
            double ipLon = BitConverter.ToDouble(body, 48);
            double ipAlt = BitConverter.ToDouble(body, 56);

            //if(id == "ATS-0001")
            //{
            //    Debug.WriteLine($"항공기 {id}번 현재 위도 : {lat}, 현재 경도 : {lon}");
            //}
            
            AircraftReceived?.Invoke(id, lat, lon, alt, status, foe, ipLat, ipLon, ipAlt);
            //Debug.WriteLine($"[Aircraft] ID: {id}, Lat: {lat:F6}, Lon: {lon:F6}, Alt: {alt:F2}, IP Lat: {ipLat:F6}, IP Lon: {ipLon:F6}, IP Alt: {ipAlt:F2}, Enemy: {foe == 1}, Status: {status}");
        }

        private static void ParseLaunchCommand(byte[] body)
        {

            Debug.WriteLine("발사 수신");

            int offset = 0;

            string launchCommandId = Encoding.ASCII.GetString(body, offset, 20).Trim('\0', ' ');
            offset += 20;

            string aircraftId = Encoding.ASCII.GetString(body, offset, 8).Trim('\0', ' ');
            offset += 8;

            string missileId = Encoding.ASCII.GetString(body, offset, 8).Trim('\0', ' ');
            offset += 8;

            double impactLat = BitConverter.ToDouble(body, offset);
            offset += 8;

            double impactLon = BitConverter.ToDouble(body, offset);
            offset += 8;

            double impactAlt = BitConverter.ToDouble(body, offset);
            offset += 8;

            ImpactPointReceived?.Invoke(launchCommandId, aircraftId, missileId, impactLat, impactLon, impactAlt);

            //Debug.WriteLine($"[LaunchCommand]");
            //Debug.WriteLine($"  발사명령 식별자: {launchCommandId}");
            //Debug.WriteLine($"  항공기 식별자  : {aircraftId}");
            //Debug.WriteLine($"  미사일 식별자  : {missileId}");
            //Debug.WriteLine($"  목표 위도      : {impactLat}°");
            //Debug.WriteLine($"  목표 경도      : {impactLon}°");
            //Debug.WriteLine($"  목표 고도      : {impactAlt} km");
        }

        private static void ParseMissile(byte[] body)
        {
            string id = Encoding.ASCII.GetString(body, 0, 8).Trim('\0');
            uint status = BitConverter.ToUInt32(body, 8);
            double lat = BitConverter.ToDouble(body, 12);
            double lon = BitConverter.ToDouble(body, 20);
            double alt = BitConverter.ToDouble(body, 28);

            MissileReceived?.Invoke(id, lat, lon, alt, status);

            //if (id == "MSS-100")
            //{
            //    Debug.WriteLine($"미사일 {id}번 현재 위도 : {lat}, 현재 경도 : {lon}");
            //}
            //Debug.WriteLine($"[Missile] ID: {id}, Lat: {lat:F6}, Lon: {lon:F6}, Alt: {alt:F2}, Status: {status}");
        }

        private static void ParseMissileDestroy(byte[] body)
        {
            // body: [발사명령식별자(20)][항공기식별자(8)][미사일식별자(8)][폭파종류(4)]
            int offset = 0;

            string launchCommandId = Encoding.ASCII.GetString(body, offset, 20).Trim('\0', ' ');
            offset += 20;

            string aircraftId = Encoding.ASCII.GetString(body, offset, 8).Trim('\0', ' ');
            offset += 8;

            string missileId = Encoding.ASCII.GetString(body, offset, 8).Trim('\0', ' ');
            offset += 8;

            uint destroyType = BitConverter.ToUInt32(body, offset);
            offset += 4;

            // 필요에 따라 이벤트로 전달 (destroyType 추가)
            MissileDestroyReceived?.Invoke(launchCommandId, aircraftId, missileId, destroyType);

            Debug.WriteLine($"[MissileDestroy]");
            Debug.WriteLine($"  발사명령 식별자: {launchCommandId}");
            Debug.WriteLine($"  항공기 식별자  : {aircraftId}");
            Debug.WriteLine($"  미사일 식별자  : {missileId}");
            Debug.WriteLine($"  폭파 종류      : {destroyType}");
        }

    }

}