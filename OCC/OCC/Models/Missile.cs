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
            SelfDestroy = 4, // 자폭
            FollowUp = 5 //유도
        }

        public string Id { get; }  // 식별자는 불변
        private double latitude { get; set; }
        private double longitude { get; set; }
        private double altitude { get; set; }

        private bool hasLaunched = false; // launching.gif를 1번만 보여주기 위한 플래그

        private uint status;
        public uint Status
        {
            get => status;
            set
            {
                var old = status; 
                var newVal = value;

                if (old != newVal)
                {
                    switch (newVal)
                    {
                        case 0: // 대기 상태
                            VisualState = MissileVisualState.Waiting;
                            break;

                        case 1: // 비행 중
                            if (old == 0 && !hasLaunched)
                            {
                                VisualState = MissileVisualState.Launching;
                                hasLaunched = true;
                            }
                            else
                            {
                                VisualState = MissileVisualState.InFlight;
                            }
                            break;

                        //case 2:  // 2: 명중 성공  
                        case 5: // 시뮬레이터와 .gif 동기화를 위해, 종말 유도 모드 5로 수정
                            if (old == 1)
                                VisualState = MissileVisualState.HitSuccess;
                            else
                                VisualState = MissileVisualState.Done;
                            break;

                        case 3: // 비상 폭파
                            if (old == 1)
                                VisualState = MissileVisualState.EmergencyExplode;
                            else
                                VisualState = MissileVisualState.Done;
                            break;

                        case 4: // 자폭
                            if (old == 1)
                                VisualState = MissileVisualState.SelfExplode;
                            else
                                VisualState = MissileVisualState.Done;
                            break;

                        default:
                            //VisualState = MissileVisualState.Done;  // X
                            break;
                    }
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
                if (visualState != value)
                {
                    visualState = value;
                    OnPropertyChanged(nameof(VisualState));  // ← 이거 중요!
                }
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
            MissileStatus.FollowUp => "유도 중",
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
