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
using System.Diagnostics;

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

            // Loaded 이벤트를 통해 NavigationService를 설정
            Loaded += ScenarioCreatePage_Loaded;
        }

        private void ScenarioCreatePage_Loaded(object sender, RoutedEventArgs e)
        {
            if (NavigationService != null)
            {
                _viewModel.NavigationService = NavigationService;
            }
            else if (Parent is Frame frame && frame.NavigationService != null)
            {
                _viewModel.NavigationService = frame.NavigationService;
            }
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
                    latitude = position.Lat,
                    longitude = position.Lng,
                    altitude = 10.0
                };

                if (_viewModel.HandleMapClick(coordinate))
                {
                    // 마커 추가
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

                    // 시작과 끝점 연결
                    if (_viewModel.StartPoint != null && _viewModel.EndPoint != null)
                    {
                        Debug.WriteLine($"StartPoint: {_viewModel.StartPoint?.latitude}, {_viewModel.StartPoint?.longitude}");
                        Debug.WriteLine($"EndPoint: {_viewModel.EndPoint?.latitude}, {_viewModel.EndPoint?.longitude}");
                        // WPF의 PathGeometry를 사용하여 선 생성
                        var pathGeometry = new PathGeometry();
                        var pathFigure = new PathFigure
                        {
                            StartPoint = new Point(_viewModel.StartPoint.longitude, _viewModel.StartPoint.latitude),
                            Segments = new PathSegmentCollection
                            {
                                new LineSegment(new Point(_viewModel.EndPoint.longitude, _viewModel.EndPoint.latitude), true)
                            }
                        };
                        pathGeometry.Figures.Add(pathFigure);

                        // GMapMarker로 변환하여 추가
                        GMapMarker routeMarker = new GMapMarker(new PointLatLng(_viewModel.StartPoint.latitude, _viewModel.StartPoint.longitude))
                        {
                            Shape = new Path
                            {
                                Data = pathGeometry,
                                Stroke = _viewModel.GetMarkerColor(),
                                StrokeThickness = 2
                            }
                        };

                        mapControl.Markers.Add(routeMarker);
                    }
                }
            }
            // 이벤트 전파 차단 -> 다른 컨트롤에 이벤트가 전달되지 않도록 함(한번 클릭해도 두번 눌리는 현상 방지)
            e.Handled = true;
        }
    }
}
