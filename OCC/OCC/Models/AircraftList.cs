using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Models
{
    public class AircraftWithIp
    {
        public string AircraftId { get; set; } // 최대 8자
        public double Latitude { get; set; }
        public double Longitude { get; set; }
        public double Altitude { get; set; }
        public bool IsEnemy { get; set; }
        public uint Status { get; set; }
        public double IpLatitude { get; set; }
        public double IpLongitude { get; set; }
        public double IpAltitude { get; set; }

        public AircraftWithIp(string aircraftId, double latitude, double longitude, double altitude, bool isEnemy, uint status, double ipLatitude, double ipLongitude, double ipAltitude)
        {
            AircraftId = aircraftId.Length > 8 ? aircraftId.Substring(0, 8) : aircraftId;
            Latitude = latitude;
            Longitude = longitude;
            Altitude = altitude;
            IsEnemy = isEnemy;
            Status = status;
            IpLatitude = ipLatitude;
            IpLongitude = ipLongitude;
            IpAltitude = ipAltitude;
        }
    }

    public class AircraftList
    {
        public ObservableCollection<AircraftWithIp> Aircrafts { get; } = new ObservableCollection<AircraftWithIp>();
    }
}
