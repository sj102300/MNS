using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Text.Json.Serialization;

namespace Aircraft_Visual
{
    public class Point
    {
        [JsonPropertyName("latitude")]
        public double Latitude { get; set; }

        [JsonPropertyName("longitude")]
        public double Longitude { get; set; }

        [JsonPropertyName("altitude")]
        public double Altitude { get; set; }
    }
    public class Aircraft
    {
        [JsonPropertyName("aircraft_id")]
        public string AircraftId { get; set; }

        [JsonPropertyName("start_point")]
        public Point StartPoint { get; set; }

        [JsonPropertyName("end_point")]
        public Point EndPoint { get; set; }

        [JsonPropertyName("friend_or_foe")]
        public string FriendOrFoe { get; set; }
    }

    /// <summary>
    /// MainWindow.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            _ = InitializeDataAsync();
        }

        private async Task InitializeDataAsync()
        {
            List<Aircraft> aircrafts = await AirInfoReciever();
            foreach (var aircraft in aircrafts)
            {
                if (aircraft.FriendOrFoe == "E")
                {
                    AirPlanePosition(aircraft.StartPoint.Latitude, aircraft.StartPoint.Longitude);
                }
            }
        }

        private void AirPlanePosition(double latitude, double longitude)
        {
            // WPF Canvas에서는 좌표를 설정할 때 x, y 좌표를 사용
            double x = ConvertLongitudeToCanvasX(longitude);
            double y = ConvertLatitudeToCanvasY(latitude);

            Image enemyAirplane = new Image
            {
                Source = new BitmapImage(new Uri("C:\\Users\\user\\Desktop\\NorthKAir.png")),
                Width = 50,
                Height = 50
            };

            // 이미지를 캔버스에 추가하고 좌표 설정
            EnemyAirforce.Children.Add(enemyAirplane);
            Canvas.SetLeft(enemyAirplane, x);
            Canvas.SetTop(enemyAirplane, y);
        }

        private double ConvertLongitudeToCanvasX(double longitude)
        {
            // 경도를 캔버스 x 좌표로 변환하는 로직
            // 실제 변환 로직은 필요에 따라 조정해야 합니다.
            return (longitude - 126) * 10; // 예시 변환
        }

        private double ConvertLatitudeToCanvasY(double latitude)
        {
            // 위도를 캔버스 y 좌표로 변환하는 로직
            // 실제 변환 로직은 필요에 따라 조정해야 합니다.
            return (37 - latitude) * 10; // 예시 변환
        }

        private async Task<List<Aircraft>> AirInfoReciever() //비동기를 위해 Task 사용
        {
            // 실제로 TCP통신을 통해 TCC로 부터 데이터를 받아 비행기 좌표를 계속 업데이트 해야 함
            // buffer를 통해 나에게 전달(?)
            
            // 지금은 더미데이터를 사용
            List<byte> data = new List<byte>();

            // ATS-0001
            data.AddRange(Encoding.ASCII.GetBytes("ATS-0001".PadRight(10, '\0'))); // ID
            data.AddRange(BitConverter.GetBytes(37.123456)); // Start Latitude
            data.AddRange(BitConverter.GetBytes(127.123456)); // Start Longitude
            data.AddRange(BitConverter.GetBytes(10.0)); // Start Altitude
            data.AddRange(BitConverter.GetBytes(36.654321)); // End Latitude
            data.AddRange(BitConverter.GetBytes(128.654321)); // End Longitude
            data.AddRange(BitConverter.GetBytes(10.0)); // End Altitude
            data.Add((byte)'E'); // Friend or Foe

            // ATS-0002
            data.AddRange(Encoding.ASCII.GetBytes("ATS-0002".PadRight(10, '\0'))); // ID
            data.AddRange(BitConverter.GetBytes(37.111111)); // Start Latitude
            data.AddRange(BitConverter.GetBytes(127.111111)); // Start Longitude
            data.AddRange(BitConverter.GetBytes(10.0)); // Start Altitude
            data.AddRange(BitConverter.GetBytes(36.666666)); // End Latitude
            data.AddRange(BitConverter.GetBytes(128.666666)); // End Longitude
            data.AddRange(BitConverter.GetBytes(10.0)); // End Altitude
            data.Add((byte)'O'); // Friend or Foe

            byte[] AircraftData = data.ToArray();
            return await Task.FromResult(ParseAircraftData(AircraftData)); // 실제 통신 할 때는 비동기적 처리 해야 함
        }


        private List<Aircraft> ParseAircraftData(byte[] data)
        {
            List<Aircraft> aircrafts = new List<Aircraft>();
            int offset = 0;
            int recordSize = 10 + 3 * 8 + 3 * 8 + 1; // ID(10) + 3*double(8) + 3*double(8) + 1

            while (offset < data.Length)
            {
                Aircraft aircraft = new Aircraft
                {
                    AircraftId = Encoding.ASCII.GetString(data, offset, 10).TrimEnd('\0'),
                    StartPoint = new Point
                    {
                        Latitude = BitConverter.ToDouble(data, offset + 10),
                        Longitude = BitConverter.ToDouble(data, offset + 18),
                        Altitude = BitConverter.ToDouble(data, offset + 26)
                    },
                    EndPoint = new Point
                    {
                        Latitude = BitConverter.ToDouble(data, offset + 34),
                        Longitude = BitConverter.ToDouble(data, offset + 42),
                        Altitude = BitConverter.ToDouble(data, offset + 50)
                    },
                    FriendOrFoe = Encoding.ASCII.GetString(data, offset + 58, 1)
                };

                aircrafts.Add(aircraft);
                offset += recordSize;
            }

            return aircrafts;
        }

    }
}
