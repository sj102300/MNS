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

namespace OCC.Utils
{
    public static class UdpReceiver
    {
        public static void Start(AircraftList aircraftList)
        {
            Task.Run(() =>
            {
                IPEndPoint ep = new IPEndPoint(IPAddress.Parse("192.168.2.194"), 9001);
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
                        //ParseMissile();
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
                    list.Aircrafts.Add(new AircraftWithIp(id, lat, lon, alt, foe == 1, status, ipLat, ipLon, ipAlt));
                }
                Debug.WriteLine($"[Aircraft] ID: {id}, Lat: {lat:F6}, Lon: {lon:F6}, Alt: {alt:F2}, IP Lat: {ipLat:F6}, IP Lon: {ipLon:F6}, IP Alt: {ipAlt:F2}, Enemy: {foe == 1}, Status: {status}");
            }
        }
        
        //private void ParseMissile(byte[] body)
        //{
        //    string id = Encoding.ASCII.GetString(body, 0, 8).Trim('\0');
        //    double lat = BitConverter.ToDouble(body, 8);
        //    double lon = BitConverter.ToDouble(body, 16);
        //    double alt = BitConverter.ToDouble(body, 24);
        //    uint status = BitConverter.ToUInt32(body, 32);

        //    var found = missileList.FirstOrDefault(m => m.Id == id);
        //    if (found != null)
        //    {
        //        found.Latitude = lat;
        //        found.Longitude = lon;
        //        found.Altitude = alt;
        //        found.Status = status;
        //    }
        //    else
        //    {
        //        missileList.Add(new Missile
        //        {
        //            Id = id,
        //            Latitude = lat,
        //            Longitude = lon,
        //            Altitude = alt,
        //            Status = status
        //        });
        //    }
        //}

    }

}
