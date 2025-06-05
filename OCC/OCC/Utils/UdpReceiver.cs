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


namespace OCC.Utils
{
    public static class UdpReceiver
    {
        public static void Start(AircraftList aircraftList, MissileList missileList)
        {
            Task.Run(() =>
            {
                IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.88"), 9001);        //승엽
                //IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.194"), 9001);       //승주
                //IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.195"), 9001);       //명준
                //IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.200"), 9999);
                using var udp = new UdpClient(ep);

                Debug.WriteLine("항공기 정보 수신 시작");

                while (true)
                {
                    var remote = new IPEndPoint(IPAddress.Any, 0);
                    byte[] data = udp.Receive(ref remote);

                    uint cmd = BitConverter.ToUInt32(data, 0);
                    if (cmd == 100)
                    {
                        ParseAircraft(data.Skip(8).ToArray(), aircraftList);
                    }
                    else if (cmd == 300)
                    {
                        ParseMissile(data.Skip(8).ToArray(), missileList);
                    }
                }
            });
        }

        private static void ParseAircraft(byte[] body, AircraftList list)
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

            lock (list.Aircrafts)
            {
                var existing = list.Aircrafts.FirstOrDefault(a => a.AircraftId == id);
                if (existing != null)
                {
                    existing.Latitude = lat;
                    existing.Longitude = lon;
                    existing.Altitude = alt;
                    existing.IsEnemy = foe == 1;
                    existing.Status = status;
                    existing.IpLatitude = ipLat;
                    existing.IpLongitude = ipLon;
                    existing.IpAltitude = ipAlt;
                }
                else
                {
                    Application.Current.Dispatcher.BeginInvoke(new Action(() =>
                    {
                       list.Aircrafts.Add(new AircraftWithIp(id, lat, lon, alt, foe == 1, status, ipLat, ipLon, ipAlt));
                    }));
                }
                Debug.WriteLine($"[Aircraft] ID: {id}, Lat: {lat:F6}, Lon: {lon:F6}, Alt: {alt:F2}, IP Lat: {ipLat:F6}, IP Lon: {ipLon:F6}, IP Alt: {ipAlt:F2}, Enemy: {foe == 1}, Status: {status}");
            }
        }
        
        private static void ParseMissile(byte[] body, MissileList list)
        {
            string id = Encoding.ASCII.GetString(body, 0, 8).Trim('\0');
            uint status = BitConverter.ToUInt32(body, 8);
            double lat = BitConverter.ToDouble(body, 12);
            double lon = BitConverter.ToDouble(body, 20);
            double alt = BitConverter.ToDouble(body, 28);

            lock (list.Missiles)
            {
                var existing = list.Missiles.FirstOrDefault(a => a.MissileId == id);
                if (existing != null)
                {
                    existing.Latitude = lat;
                    existing.Longitude = lon;
                    existing.Altitude = alt;
                    existing.Status = status;
                }
                else
                {
                    Application.Current.Dispatcher.BeginInvoke(new Action(() =>
                    {
                        list.Missiles.Add(new Missile(id, lat, lon, alt, status));
                    }));
                }
                Debug.WriteLine($"[Missile] ID: {id}, Lat: {lat:F6}, Lon: {lon:F6}, Alt: {alt:F2}, Status: {status}");
            }
        }

    }

}
