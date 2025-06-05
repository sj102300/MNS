using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Models
{
    public class AircraftWithIp : INotifyPropertyChanged
    {
        public enum AircraftStatus
        {
            NotEngageable = 0,  // 교전 불가능
            Engageable = 1, // 교전 가능
            Engaging = 2,   // 교전 중
            Success = 3,  //격추 성공
        }

        public string Id { get; }  // 식별자는 불변

        private double latitude;
        private double longitude;
        private double altitude;
        private uint status;
        private double ip_latitude;
        private double ip_longitude;
        private double ip_altitude;

        public string AircraftStatusText
        {
            get
            {
                return Status switch
                {
                    0 => "교전 불가능",
                    1 => "교전 가능",
                    2 => "교전 중",
                    3 => "교전 성공",
                    _ => "알 수 없음"
                };
            }
        }

        public double Latitude { get => latitude; set { latitude = value; OnPropertyChanged(nameof(Latitude)); } }
        public double Longitude { get => longitude; set { longitude = value; OnPropertyChanged(nameof(Longitude)); } }
        public double Altitude { get => altitude; set { altitude = value; OnPropertyChanged(nameof(Altitude)); } }
        public uint Status
        {
            get => status; set
            {
                status = value; OnPropertyChanged(nameof(Status));
                OnPropertyChanged(nameof(AircraftStatusText)); // 상태 텍스트도 갱신
            }
        }

        public double IpLatitude
        {
            get => ip_latitude;
            set
            {
                if (ip_latitude != value)
                {
                    ip_latitude = value;
                    OnPropertyChanged(nameof(IpLatitude));
                }
            }
        }

        public double IpLongitude
        {
            get => ip_longitude;
            set
            {
                if (ip_longitude != value)
                {
                    ip_longitude = value;
                    OnPropertyChanged(nameof(IpLongitude));
                }
            }
        }

        public double IpAltitude
        {
            get => ip_altitude;
            set
            {
                if (ip_altitude != value)
                {
                    ip_altitude = value;
                    OnPropertyChanged(nameof(IpAltitude));
                }
            }
        }

        public AircraftWithIp(string id) => Id = id;

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }

}
