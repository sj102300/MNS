using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

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
        private double latitude { get; set; }
        private double longitude { get; set; }
        private double altitude { get; set; }

        private uint status;
        public uint Status
        {
            get => status;
            set
            {
                var old = status; 
                var newVal = value;

                if(old != newVal)
                {
                    if (old == 0 && newVal == 1)
                        VisualState = MissileVisualState.Launching;
                    else if (old == 1 && newVal == 3)
                        VisualState = MissileVisualState.PressingButton;
                    else
                        VisualState = (MissileVisualState)newVal;

                    //OnPropertyChanged(nameof(VisualState));
                    //Debug.Write("VisualState Changed");
                }

                status = newVal;
                OnPropertyChanged(nameof(Status));
                OnPropertyChanged(nameof(MissileStatusText));
            }
        }
        private MissileVisualState visualState;
        public MissileVisualState VisualState
        {
            get => visualState;
            set
            {
                visualState = value;
                OnPropertyChanged(nameof(VisualState));
            }
        }

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

        public string MissileStatusText => ((MissileStatus)Status) switch
        {
            MissileStatus.BeforeLaunch => "발사전",
            MissileStatus.InFlight => "비행중",
            MissileStatus.HitSuccess => "명중성공",
            MissileStatus.EmergencyDestroy => "비상폭파",
            MissileStatus.SelfDestroy => "자폭",
            _ => "알 수 없음"
        };

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));

        public Missile(string id)
        {
            Id = id;
            Status = 0;
            VisualState = MissileVisualState.Waiting;
        }
    }
}
