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

        public static event Action<string, double, double, double, uint, double, double, double> AircraftReceived;
        public static event Action<string, double, double, double, uint> MissileReceived;

        public static void Start(ObservableCollection<AircraftWithIp> aircraftList, Dictionary<string, AircraftWithIp> aircraftLookup,
            ObservableCollection<Missile> missileList, Dictionary<string, Missile> missileLookup)
        {
            cts = new CancellationTokenSource();
            var token = cts.Token;

            Task.Run(() =>
            {
                //IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.88"), 9001);        //승엽
                //IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.194"), 9001);       //승주
                IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.195"), 9001);       //명준
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
            uint foe = BitConverter.ToUInt32(body, 32);
            uint status = BitConverter.ToUInt32(body, 36);
            double ipLat = BitConverter.ToDouble(body, 40);
            double ipLon = BitConverter.ToDouble(body, 48);
            double ipAlt = BitConverter.ToDouble(body, 56);

            AircraftReceived?.Invoke(id, lat, lon, alt, status, ipLat, ipLon, ipAlt);
            //Debug.WriteLine($"[Aircraft] ID: {id}, Lat: {lat:F6}, Lon: {lon:F6}, Alt: {alt:F2}, IP Lat: {ipLat:F6}, IP Lon: {ipLon:F6}, IP Alt: {ipAlt:F2}, Enemy: {foe == 1}, Status: {status}");
        }

        private static void ParseMissile(byte[] body)
        {
            string id = Encoding.ASCII.GetString(body, 0, 8).Trim('\0');
            uint status = BitConverter.ToUInt32(body, 8);
            double lat = BitConverter.ToDouble(body, 12);
            double lon = BitConverter.ToDouble(body, 20);
            double alt = BitConverter.ToDouble(body, 28);

            MissileReceived?.Invoke(id, lat, lon, alt, status);
            //Debug.WriteLine($"[Missile] ID: {id}, Lat: {lat:F6}, Lon: {lon:F6}, Alt: {alt:F2}, Status: {status}");
        }

    }

}