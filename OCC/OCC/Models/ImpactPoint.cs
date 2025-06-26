using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Models
{
    public class ImpactPoint : INotifyPropertyChanged
    {
        public string Id { get; }  // 식별자는 불변

        private double latitude;
        private double longitude;
        private double altitude;
        private string missileId;
        private string aircraftId;
        public double Latitude { get => latitude; set { latitude = value; OnPropertyChanged(nameof(Latitude)); } }
        public double Longitude { get => longitude; set { longitude = value; OnPropertyChanged(nameof(Longitude)); } }
        public double Altitude { get => altitude; set { altitude = value; OnPropertyChanged(nameof(Altitude)); } }
        public string MissileId
        {
            get => missileId;
            set
            {
                missileId = value;
                OnPropertyChanged(nameof(MissileId));
            }
        }

        public string AircraftId
        {
            get => aircraftId;
            set
            {
                aircraftId = value;
                OnPropertyChanged(nameof(AircraftId));
            }
        }
        public ImpactPoint(string id) => Id = id;

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
