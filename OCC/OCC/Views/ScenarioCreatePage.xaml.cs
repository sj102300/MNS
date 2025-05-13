using GMap.NET.MapProviders;
using GMap.NET.WindowsPresentation;
using GMap.NET;
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
using OCC.ViewModels;
using OCC.Models;

namespace OCC.Views
{
    /// <summary>
    /// ScenarioCreatePage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ScenarioCreatePage : Page
    {
        //PointLatLng start;
        //PointLatLng end;

        //marker
        //GMapMarker currentMarker;

        //zones list
        //List<GMapMarker> Circles = new List<GMapMarker>();

        private Point _mouseDownPoint;
        private DateTime _mouseDownTime;
        private const double CLICK_THRESHOLD = 5.0; // 픽셀 거리
        private const int CLICK_TIME_MS = 300; // 클릭 시간 제한 (밀리초)

        private ScenarioCreateViewModel _viewModel;
        public ScenarioCreatePage()
        {
            InitializeComponent();
            _viewModel = new ScenarioCreateViewModel();
            DataContext = _viewModel;
            InitializeMap();
        }

        //Position : 위/경도 정보 -> 입력 순서는 경도, 위도 순서
        private void InitializeMap()
        {
            // config map
            mapControl.MapProvider = GMapProviders.GoogleMap;
            mapControl.Position = new PointLatLng(37.5665, 126.9780);
            mapControl.MinZoom = 2;
            mapControl.MaxZoom = 18;
            mapControl.Zoom = 12;
            mapControl.ShowCenter = false;
            mapControl.CanDragMap = true;
            mapControl.MouseWheelZoomType = MouseWheelZoomType.MousePositionAndCenter;
            mapControl.DragButton = MouseButton.Left;
            mapControl.MouseLeftButtonDown += mapControl_MouseLeftButtonDown;
            mapControl.MouseLeftButtonUp += mapControl_MouseLeftButtonUp;
        }

        private void mapControl_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _mouseDownPoint = e.GetPosition(mapControl);
            _mouseDownTime = DateTime.Now;
        }

        private void mapControl_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            Point mouseUpPoint = e.GetPosition(mapControl);
            TimeSpan elapsed = DateTime.Now - _mouseDownTime;
            double distance = (mouseUpPoint - _mouseDownPoint).Length;

            if (distance < CLICK_THRESHOLD && elapsed.TotalMilliseconds < CLICK_TIME_MS)
            {
                // 진짜 클릭으로 간주
                PointLatLng position = mapControl.FromLocalToLatLng((int)mouseUpPoint.X, (int)mouseUpPoint.Y);

                var coordinate = new Coordinate
                {
                    Latitude = position.Lat,
                    Longitude = position.Lng,
                    Altitude = 10.0
                };

                if (_viewModel.HandleMapClick(coordinate))
                {
                    GMapMarker marker = new GMapMarker(position)
                    {
                        Shape = new Ellipse
                        {
                            Width = 15,
                            Height = 15,
                            Stroke = Brushes.Black,
                            StrokeThickness = 2,
                            Fill = _viewModel.GetMarkerColor()
                        }
                    };
                    mapControl.Markers.Add(marker);
                }

                // 이벤트 전파 차단 -> 다른 컨트롤에 이벤트가 전달되지 않도록 함(한번 클릭해도 두번 눌리는 현상 방지)
                e.Handled = true;
            }
            //var point = e.GetPosition(mapControl);
            //PointLatLng position = mapControl.FromLocalToLatLng((int)point.X, (int)point.Y);

            //// ViewModel의 MapClickCommand 호출
            //var coordinate = new Coordinate
            //{
            //    Latitude = position.Lat,
            //    Longitude = position.Lng,
            //    Altitude = 10.0
            //};

            //// ViewModel에서 점을 찍을 수 있는지 확인
            //if (_viewModel.HandleMapClick(coordinate))
            //{
            //    // 지도에 마커 추가
            //    GMapMarker marker = new GMapMarker(position)
            //    {
            //        Shape = new System.Windows.Shapes.Ellipse
            //        {
            //            Width = 10,
            //            Height = 10,
            //            Stroke = Brushes.Black,
            //            StrokeThickness = 2,
            //            Fill = _viewModel.GetMarkerColor() // ViewModel에서 색상 가져오기
            //        }
            //    };
            //    mapControl.Markers.Add(marker);
            //}
        }

    }
}
