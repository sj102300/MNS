using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Models
{
    public class Missile : INotifyPropertyChanged
    {
        public enum MissileStatus
        {
            BeforeLaunch = 0,  // 발사전
            InFlight = 1, // 비행중
            HitSuccess = 2,   // 명중성공
            EmergencyDestroy = 3,  // 비상폭파
            SelfDestroy = 4 // 자폭
        }

        public string Id { get; }  // 식별자는 불변
        private double latitude;
        private double longitude;
        private double altitude;
        private uint status;
        public double Latitude
        {
            get => latitude;
            set { latitude = value; OnPropertyChanged(nameof(Latitude)); }
        }
        public double Longitude
        {
            get => longitude;
            set { longitude = value; OnPropertyChanged(nameof(Longitude)); }
        }
        public double Altitude
        {
            get => altitude;
            set { altitude = value; OnPropertyChanged(nameof(Altitude)); }
        }
        public uint Status
        {
            get => status;
            set
            {
                status = value; OnPropertyChanged(nameof(Status));
                OnPropertyChanged(nameof(MissileStatusText)); // 상태 텍스트도 갱신
            }
        }
        public string MissileStatusText
        {
            get
            {
                return (MissileStatus)Status switch
                {
                    MissileStatus.BeforeLaunch => "발사전",
                    MissileStatus.InFlight => "비행중",
                    MissileStatus.HitSuccess => "명중성공",
                    MissileStatus.EmergencyDestroy => "비상폭파",
                    MissileStatus.SelfDestroy => "자폭",
                    _ => "알 수 없음"
                };
            }
        }
        public Missile(string id)
        {
            Id = id;
        }
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }
    }
}
