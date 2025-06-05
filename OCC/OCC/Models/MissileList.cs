using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Models
{
    public class Missile
    {
        public string MissileId { get; set; } // 최대 8자
        public double Latitude { get; set; }
        public double Longitude { get; set; }
        public double Altitude { get; set; }
        public uint Status { get; set; }

        public Missile(string missileId, double latitude, double longitude, double altitude, uint status)
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
        public ObservableCollection<Missile> Missiles { get; } = new ObservableCollection<Missile>();
        public void AddMissile(Missile missile)
        {
            Missiles.Add(missile);
        }
    }
}