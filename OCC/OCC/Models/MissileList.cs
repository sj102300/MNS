using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Models
{
    public class Missile
    {
        public string MissileId { get; set; } // 최대 8자
        double Latitude { get; set; }
        double Longitude { get; set; }
        double Altitude { get; set; }
        public int Status { get; set; }

        public Missile(string missileId, double latitude, double longitude, double altitude, int status)
        {
            MissileId = missileId.Length > 8 ? missileId.Substring(0, 8) : missileId;
            Latitude = latitude;
            Longitude = longitude;
            Altitude = altitude;
            Status = status;
        }
    }
    public class MissileList
    {
        public List<Missile> Missiles { get; } = new List<Missile>();
        //    public List<AircraftWithIp> Aircrafts { get; } = new List<AircraftWithIp>();
        public void AddMissile(Missile missile)
        {
            Missiles.Add(missile);
        }
        public void RemoveMissile(string missileId)
        {
            Missiles.RemoveAll(m => m.MissileId == missileId);
        }
    }
}