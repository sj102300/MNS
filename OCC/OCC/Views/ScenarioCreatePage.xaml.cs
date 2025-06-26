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

            // GMapControl에 포커스 강제 부여
            mapControl.Focus();
        }

        //Position : 위/경도 정보 -> 입력 순서는 경도, 위도 순서
        private void InitializeMap()
        {
            // config map
            mapControl.MapProvider = GMapProviders.GoogleMap;
            mapControl.Position = new PointLatLng(37.5665, 126.9780);
            mapControl.MinZoom = 2;
            mapControl.MaxZoom = 18;
            mapControl.Zoom = 7;
            mapControl.ShowCenter = false;
            mapControl.CanDragMap = true;
            mapControl.MouseWheelZoomType = MouseWheelZoomType.MousePositionAndCenter;
            mapControl.DragButton = MouseButton.Left;
            mapControl.MouseLeftButtonDown += mapControl_MouseLeftButtonDown;
            mapControl.MouseLeftButtonUp += mapControl_MouseLeftButtonUp;

            mapControl.PreviewMouseWheel += (s, e) =>
            {
                if (e.Delta > 0)
                    mapControl.Zoom = Math.Min(mapControl.Zoom + 1, mapControl.MaxZoom);
                else
                    mapControl.Zoom = Math.Max(mapControl.Zoom - 1, mapControl.MinZoom);
                e.Handled = true;
            };
        }

        private GMapPolygon CreateCircle(PointLatLng center, double radiusKm, int segments, Brush stroke, Brush fill = null)
        {
            var points = new List<PointLatLng>();
            double seg = 360.0 / segments;
            for (int i = 0; i < segments; i++)
            {
                double theta = Math.PI * i * seg / 180.0;
                // 위도 1도 ≈ 111.32km, 경도 1도 ≈ 111.32km * cos(위도)
                double dLat = (radiusKm / 111.32) * Math.Sin(theta);
                double dLng = (radiusKm / (111.32 * Math.Cos(center.Lat * Math.PI / 180))) * Math.Cos(theta);
                points.Add(new PointLatLng(center.Lat + dLat, center.Lng + dLng));
            }
            var polygon = new GMapPolygon(points);

            // WPF Path 스타일 적용
            polygon.Shape = new System.Windows.Shapes.Path
            {
                Stroke = stroke,
                StrokeThickness = 3,
                Fill = fill ?? Brushes.Transparent,
                Opacity = 0.4
            };

            return polygon;
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
                    double markerSize = 15;
                    string markerText = (_viewModel.EndPoint == null) ? "S" : "E"; // 첫번째는 S, 두번째는 E

                    var markerShape = new Grid
                    {
                        Width = markerSize,
                        Height = markerSize
                    };
                    markerShape.Children.Add(new Ellipse
                    {
                        Width = markerSize,
                        Height = markerSize,
                        Stroke = Brushes.Black,
                        StrokeThickness = 2,
                        Fill = _viewModel.GetMarkerColor()
                    });
                    markerShape.Children.Add(new TextBlock
                    {
                        Text = markerText,
                        Foreground = Brushes.White,
                        FontWeight = FontWeights.Bold,
                        FontSize = 13,
                        HorizontalAlignment = HorizontalAlignment.Center,
                        VerticalAlignment = VerticalAlignment.Center,
                        TextAlignment = TextAlignment.Center
                    });

                    GMapMarker marker = new GMapMarker(position)
                    {
                        Shape = markerShape,
                        Offset = new Point(-markerSize / 2, -markerSize / 2)
                    };
                    mapControl.Markers.Add(marker);

                    if(_viewModel.SelectedItem == "포대")
                    {
                        // 300km, 130km 원 추가
                        var circle300 = CreateCircle(position, 300, 120, Brushes.Red, null);
                        var circle130 = CreateCircle(position, 130, 120, Brushes.ForestGreen, null);
                        mapControl.RegenerateShape(circle300);
                        mapControl.RegenerateShape(circle130);
                        mapControl.Markers.Add(circle300);
                        mapControl.Markers.Add(circle130);
                    }

                    // 시작과 끝점 연결
                    if (_viewModel.StartPoint != null && _viewModel.EndPoint != null)
                    {
                        Debug.WriteLine($"StartPoint: {_viewModel.StartPoint?.latitude}, {_viewModel.StartPoint?.longitude}");
                        Debug.WriteLine($"EndPoint: {_viewModel.EndPoint?.latitude}, {_viewModel.EndPoint?.longitude}");

                        // 마커 반지름(픽셀)
                        double markerRadiusPx = markerSize / 2.0;

                        // 지도상에서 1픽셀이 몇 미터인지 계산
                        // (지도 확대/축소에 따라 달라짐)
                        double metersPerPixel = mapControl.MapProvider.Projection.GetGroundResolution(
                            (int)mapControl.Zoom, position.Lat);

                        // 시작점과 끝점
                        var startLatLng = new PointLatLng(_viewModel.StartPoint.latitude, _viewModel.StartPoint.longitude);
                        var endLatLng = new PointLatLng(_viewModel.EndPoint.latitude, _viewModel.EndPoint.longitude);

                        // 방향 벡터(단위 벡터)
                        double dx = endLatLng.Lng - startLatLng.Lng;
                        double dy = endLatLng.Lat - startLatLng.Lat;
                        double length = Math.Sqrt(dx * dx + dy * dy);
                        if (length == 0) length = 1; // 0 division 방지
                        dx /= length;
                        dy /= length;

                        // 마커 반지름만큼 이동(위/경도 단위)
                        double offsetMeter = markerRadiusPx * metersPerPixel;

                        // 위도 1도는 약 111,320m, 경도 1도는 위도에 따라 다름
                        double latOffset = (offsetMeter / 111320.0) * dy;
                        double lngOffset = (offsetMeter / (111320.0 * Math.Cos(startLatLng.Lat * Math.PI / 180))) * dx;

                        // 시작점/끝점에서 offset만큼 이동한 좌표
                        var startEdge = new PointLatLng(startLatLng.Lat + latOffset, startLatLng.Lng + lngOffset);
                        var endEdge = new PointLatLng(endLatLng.Lat - latOffset, endLatLng.Lng - lngOffset);

                        // GMapRoute를 사용해 선 그리기 (테두리~테두리)
                        var points = new List<PointLatLng> { startEdge, endEdge };
                        var route = new GMapRoute(points)
                        {
                            Shape = new System.Windows.Shapes.Path
                            {
                                Stroke = _viewModel.GetMarkerColor(),
                                StrokeThickness = 2,
                                StrokeDashArray = new DoubleCollection { 4, 4 }
                            }
                        };
                        mapControl.Markers.Add(route);            

                        // 시작점과 끝점 초기화
                        _viewModel.StartPoint = null;
                        _viewModel.EndPoint = null;
                    }
                }
            }
            // 이벤트 전파 차단 -> 다른 컨트롤에 이벤트가 전달되지 않도록 함(한번 클릭해도 두번 눌리는 현상 방지)
            e.Handled = true;
        }
    }
}
