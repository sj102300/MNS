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
            set { status = value; OnPropertyChanged(nameof(Status)); }
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
