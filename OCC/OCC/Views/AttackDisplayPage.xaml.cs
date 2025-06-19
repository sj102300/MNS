using System;
using System.Collections.Generic;
using System.Diagnostics;
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
using GMap.NET.MapProviders;
using GMap.NET.WindowsPresentation;
using GMap.NET;
using OCC.Models;
using OCC.ViewModels;
using Newtonsoft.Json;
using OCC.Utils;
using System.Net.Http;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Reflection;

namespace OCC.Views
{
    /// <summary>
    /// AttackDisplayPage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class AttackDisplayPage : Page
    {
        private AttackViewModel _viewModel;
        // 마커 추가
        private readonly Dictionary<string, GMapMarker> _aircraftMarkers = new();
        private readonly Dictionary<string, GMapMarker> _ipMarkers = new();
        private readonly Dictionary<string, GMapMarker> _missileMarkers = new();
        private readonly Dictionary<string, PointLatLng> _aircraftPrevPositions = new();
        private readonly Dictionary<string, PointLatLng> _missilePrevPositions = new();
        // 미사일 경로 표시용
        private readonly Dictionary<string, GMapRoute> _missileRoutes = new();
        private readonly Dictionary<string, List<PointLatLng>> _missileRoutePoints = new();

        private GMapMarker _batteryMarker;
        private string _scenarioId;

        // 임계값: 위경도 변화가 이 값 이상일 때만 bearing 계산 (약 5~6m)
        private const double PositionThreshold = 0.00005;

        public AttackDisplayPage(AttackViewModel viewModel, string scenarioId)
        {
            InitializeComponent();
            _viewModel = viewModel;
            DataContext = _viewModel;
            _scenarioId = scenarioId;
            // Loaded 이벤트를 통해 NavigationService를 설정
            Loaded += AttackDisplayPage_Loaded;
            InitializeMap();
            _viewModel.AircraftList.CollectionChanged += AircraftList_CollectionChanged;
            _viewModel.MissileList.CollectionChanged += MissileList_CollectionChanged;
            _viewModel.ImpactPointList.CollectionChanged += ImpactPointList_CollectionChanged;
        }

        private void ImpactPointList_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.NewItems != null)
            {
                foreach (ImpactPoint ip in e.NewItems)
                {
                    AddImageIpMarker(ip);
                    ip.PropertyChanged += ImpactPoint_PropertyChanged;
                }
            }

            if (e.OldItems != null)
            {
                foreach (ImpactPoint ip in e.OldItems)
                {
                    RemoveIpMarker(ip);
                    ip.PropertyChanged -= ImpactPoint_PropertyChanged;
                }
            }
        }

        private void AircraftList_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.NewItems != null)
            {
                foreach (AircraftWithIp aircraft in e.NewItems)
                {
                    AddImageAircraftMarker(aircraft); // 이미지 마커로 변경된 상태(테스트 필요)
                    aircraft.PropertyChanged += Aircraft_PropertyChanged;
                }
            }

            if (e.OldItems != null)
            {
                foreach (AircraftWithIp aircraft in e.OldItems)
                {
                    RemoveAircraftMarker(aircraft);
                    aircraft.PropertyChanged -= Aircraft_PropertyChanged;
                }
            }
        }

        private void Aircraft_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (sender is not AircraftWithIp aircraft) return;
            if (e.PropertyName != nameof(AircraftWithIp.Latitude) && e.PropertyName != nameof(AircraftWithIp.Longitude))
                return;

            UpdateAircraftMarkerDirection(aircraft);

            if (_aircraftMarkers.TryGetValue(aircraft.Id, out var marker))
            {
                marker.Position = new PointLatLng(aircraft.Latitude, aircraft.Longitude);
            }
        }
        private void ImpactPoint_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (sender is not ImpactPoint ip) return;
            if (e.PropertyName != nameof(ImpactPoint.Latitude) && e.PropertyName != nameof(ImpactPoint.Longitude))
                return;

            if (_ipMarkers.TryGetValue(ip.Id, out var marker))
            {
                marker.Position = new PointLatLng(ip.Latitude, ip.Longitude);
            }
        }

        //항공기 이미지 마커 변경 메서드
        private void AddImageAircraftMarker(AircraftWithIp aircraft)
        {
            double markerSize = 55;  // 아이콘 크기 조정

            // Grid로 이미지와 텍스트를 겹치게 배치
            var markerGrid = new Grid
            {
                Width = markerSize,
                Height = markerSize// 텍스트 공간 확보
            };

            // 항공기 이미지
            var image = new Image
            {
                Width = markerSize - 15,
                Height = markerSize - 15,
                Source = new BitmapImage(new Uri(
                    "pack://application:,,,/images/Aircraft.png",
                    UriKind.Absolute)),
                RenderTransformOrigin = new Point(0.5, 0.5),
                VerticalAlignment = VerticalAlignment.Top,
                HorizontalAlignment = HorizontalAlignment.Center
            };
            markerGrid.Children.Add(image);

            // 식별자 텍스트 (아래쪽에 표시)
            var text = new TextBlock
            {
                Text = aircraft.Id,
                Foreground = aircraft.Foe == 1 ? Brushes.OrangeRed : Brushes.SkyBlue,
                FontWeight = FontWeights.Bold,
                FontSize = 12,
                Background = new SolidColorBrush(Color.FromArgb(180, 30, 30, 30)),
                Padding = new Thickness(2, 0, 2, 0),
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Bottom,
                TextAlignment = TextAlignment.Center
            };
            markerGrid.Children.Add(text);

            var marker = new GMapMarker(new PointLatLng(aircraft.Latitude, aircraft.Longitude))
            {
                Shape = markerGrid,
                Offset = new Point(-markerSize / 2, -markerSize / 2)
            };

            mapControl.Markers.Add(marker);
            _aircraftMarkers[aircraft.Id] = marker;
            _aircraftPrevPositions[aircraft.Id] = new PointLatLng(aircraft.Latitude, aircraft.Longitude);
        }

        //이미지 IP 마커 변경 메서드
        private void AddImageIpMarker(ImpactPoint ip)
        {
            double markerSize = 15; // 아이콘 크기
            string imgPath = "pack://application:,,,/images/impactPoint.png";

            var image = new Image
            {
                Width = markerSize,
                Height = markerSize,
                Source = new BitmapImage(new Uri(imgPath, UriKind.Absolute)),
                RenderTransformOrigin = new Point(0.5, 0.5),
                VerticalAlignment = VerticalAlignment.Center,
                HorizontalAlignment = HorizontalAlignment.Center
            };

            var markerGrid = new Grid
            {
                Width = markerSize,
                Height = markerSize
            };
            markerGrid.Children.Add(image);

            var marker = new GMapMarker(new PointLatLng(ip.Latitude, ip.Longitude))
            {
                Shape = markerGrid,
                Offset = new Point(-markerSize / 2, -markerSize / 2)
            };

            mapControl.Markers.Add(marker);
            _ipMarkers[ip.Id] = marker;
        }

        private void RemoveAircraftMarker(AircraftWithIp aircraft)
        {
            if (_aircraftMarkers.TryGetValue(aircraft.Id, out var marker))
            {
                mapControl.Markers.Remove(marker);
                _aircraftMarkers.Remove(aircraft.Id);
            }
        }

        private void RemoveIpMarker(ImpactPoint ip)
        {
            if (_ipMarkers.TryGetValue(ip.Id, out var ipmarker))
            {
                mapControl.Markers.Remove(ipmarker);
                _ipMarkers.Remove(ip.Id);
            }
        }

        private void UpdateMissileMarkerVisibility(Missile missile)
        {
            if (_missileMarkers.TryGetValue(missile.Id, out var marker))
            {
                // 발사전이면 숨김, 그 외에는 표시
                if (missile.Status == (uint)Missile.MissileStatus.BeforeLaunch)
                    marker.Shape.Visibility = Visibility.Collapsed;
                else
                {
                    marker.Shape.Visibility = Visibility.Visible;
                    UpdateMissileMarkerDirection(missile);
                }
                    
            }
        }

        private void ReplaceMissileMarkerWithDestroyed(Missile missile, uint missile_status)
        {
            // 기존 마커 삭제
            if (_missileMarkers.TryGetValue(missile.Id, out var oldMarker))
            {
                mapControl.Markers.Remove(oldMarker);
                _missileMarkers.Remove(missile.Id);
            }

            double markerSize;
            string destroyImgPath;

            if (missile_status == (uint)Missile.MissileStatus.HitSuccess)
            {
                markerSize = 130;
                destroyImgPath = "pack://application:,,,/images/destroy.png";
            }
            else
            {
                markerSize = 100;
                destroyImgPath = "pack://application:,,,/images/emergencyDestroy.png";
            }

            var image = new Image
            {
                Width = markerSize,
                Height = markerSize,
                Source = new BitmapImage(new Uri(destroyImgPath, UriKind.Absolute)),
                RenderTransformOrigin = new Point(0.5, 0.5),
                VerticalAlignment = VerticalAlignment.Center,
                HorizontalAlignment = HorizontalAlignment.Center
            };

            var markerGrid = new Grid
            {
                Width = markerSize,
                Height = markerSize
            };
            markerGrid.Children.Add(image);

            var newMarker = new GMapMarker(new PointLatLng(missile.Latitude, missile.Longitude))
            {
                Shape = markerGrid,
                Offset = new Point(-markerSize / 2, -markerSize / 2)
            };

            mapControl.Markers.Add(newMarker);
            _missileMarkers[missile.Id] = newMarker;

            if (missile_status != (uint)Missile.MissileStatus.HitSuccess)
            {
                // 3초 후 서서히 사라지는 애니메이션 적용
                var timer = new System.Windows.Threading.DispatcherTimer
                {
                    Interval = TimeSpan.FromSeconds(3)
                };
                timer.Tick += (s, e) =>
                {
                    timer.Stop();

                    // Fade-out 애니메이션 (1초)
                    var fade = new System.Windows.Media.Animation.DoubleAnimation
                    {
                        From = 1.0,
                        To = 0.0,
                        Duration = new System.Windows.Duration(TimeSpan.FromSeconds(1)),
                        FillBehavior = System.Windows.Media.Animation.FillBehavior.Stop
                    };
                    fade.Completed += (s2, e2) =>
                    {
                        mapControl.Markers.Remove(newMarker);
                        _missileMarkers.Remove(missile.Id);
                    };
                    markerGrid.BeginAnimation(UIElement.OpacityProperty, fade);
                };
                timer.Start();
            }
        }

        private void MissileList_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (e.NewItems != null)
            {
                foreach (Missile missile in e.NewItems)
                {
                    AddImageMissileMarker(missile); // 이미지 마커로 변경된 상태(테스트 필요)
                    missile.PropertyChanged += Missile_PropertyChanged;
                    // 최초 상태에 따라 마커 표시/숨김
                    UpdateMissileMarkerVisibility(missile);
                }
            }

            if (e.OldItems != null)
            {
                foreach (Missile missile in e.OldItems)
                {
                    RemoveMissileMarker(missile);
                    missile.PropertyChanged -= Missile_PropertyChanged;
                }
            }
        }

        private void Missile_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (sender is not Missile missile) return;

            if (e.PropertyName == nameof(Missile.Latitude) || e.PropertyName == nameof(Missile.Longitude))
            {
                UpdateMissileMarkerVisibility(missile);
                if (_missileMarkers.TryGetValue(missile.Id, out var marker))
                {
                    marker.Position = new PointLatLng(missile.Latitude, missile.Longitude);
                }

                // 경로 점 추가 및 갱신
                if (_missileRoutePoints.TryGetValue(missile.Id, out var routePoints) &&
    _missileRoutes.TryGetValue(missile.Id, out var route))
                {
                    var last = routePoints.LastOrDefault();
                    var curr = new PointLatLng(missile.Latitude, missile.Longitude);

                    // bearing 계산
                    double angle = 0;
                    if (_missilePrevPositions.TryGetValue(missile.Id, out var prev))
                        angle = CalculateBearing(prev.Lat, prev.Lng, missile.Latitude, missile.Longitude);

                    var tail = GetMissileTailPosition(missile, angle, 45); // markerSize-10

                    // 중복 좌표 방지
                    if (last == null || last.Lat != curr.Lat || last.Lng != curr.Lng)
                    {
                        // 마지막 점을 꼬리로 보정
                        routePoints.Add(tail);
                        routePoints.Add(curr);
                        route.Points.Clear();
                        route.Points.AddRange(routePoints);
                        mapControl.RegenerateShape(route);

                        // 스타일 재적용
                        if (route.Shape is System.Windows.Shapes.Path path)
                        {
                            path.Stroke = Brushes.Red;
                            path.StrokeThickness = 4;
                            path.StrokeDashArray = new DoubleCollection { 6, 4 };
                            path.Opacity = 0.8;
                        }
                    }
                }
            }

            if (e.PropertyName == nameof(Missile.Status))
            {
                //UpdateMissileMarkerVisibility(missile);

                // 상태가 격추완료(HitSuccess)면 마커를 destroy.png로 교체
                if (missile.Status == (uint)Missile.MissileStatus.HitSuccess ||
                    missile.Status == (uint)Missile.MissileStatus.SelfDestroy ||
                    missile.Status == (uint)Missile.MissileStatus.EmergencyDestroy)
                {
                    ReplaceMissileMarkerWithDestroyed(missile, missile.Status);
                }
            }

            //if (e.PropertyName == nameof(Missile.Latitude) || e.PropertyName == nameof(Missile.Longitude))
            //{
            //    if (_missileMarkers.TryGetValue(missile.Id, out var marker))
            //    {
            //        marker.Position = new PointLatLng(missile.Latitude, missile.Longitude);
            //    }
            //}
        }

        // 미사일의 꼬리 위치 구하는 함수
        private PointLatLng GetMissileTailPosition(Missile missile, double angleDegree, double markerSize)
        {
            // markerSize: 이미지의 실제 픽셀 크기 (지도상에서 약간의 거리로 변환)
            // angleDegree: 미사일의 진행 방향(0=북쪽, 90=동쪽, 180=남쪽, 270=서쪽)
            // 꼬리(뒤쪽)는 진행방향의 반대(-180도)로 markerSize/2 만큼 이동

            // 위도 1도 ≈ 111,320m, 경도 1도 ≈ 111,320 * cos(위도)
            double meters = (markerSize / 2) * 2.5; // 2.5: 픽셀→미터 보정(지도 확대/축소에 따라 조정 필요)
            double bearing = (angleDegree + 180) % 360; // 꼬리 방향

            double lat = missile.Latitude;
            double lng = missile.Longitude;

            // 위도 변화량
            double dLat = meters * Math.Cos(bearing * Math.PI / 180.0) / 111320.0;
            // 경도 변화량
            double dLng = meters * Math.Sin(bearing * Math.PI / 180.0) / (111320.0 * Math.Cos(lat * Math.PI / 180.0));

            return new PointLatLng(lat + dLat, lng + dLng);
        }

        //미사일 이미지 마커 변경 메서드
        private void AddImageMissileMarker(Missile missile)
        {

            double markerSize = 55;  // 마커 이미지 크기 조정
            string imgPath = "pack://application:,,,/images/missile.png"; // 리소스 경로

            // Grid로 이미지와 텍스트를 겹치게 배치
            var markerGrid = new Grid
            {
                Width = markerSize,
                Height = markerSize // 텍스트 공간 확보
            };

            // 미사일 이미지
            var image = new Image
            {
                Width = markerSize - 10,
                Height = markerSize - 10,
                Source = new BitmapImage(new Uri(imgPath, UriKind.Absolute)),
                RenderTransformOrigin = new Point(0.5, 0.5),
                VerticalAlignment = VerticalAlignment.Top,
                HorizontalAlignment = HorizontalAlignment.Center
            };

            markerGrid.Children.Add(image);

            // 식별자 텍스트 (아래쪽에 표시)
            var text = new TextBlock
            {
                Text = missile.Id,
                Foreground = Brushes.White,
                FontWeight = FontWeights.Bold,
                FontSize = 12,
                Background = new SolidColorBrush(Color.FromArgb(180, 30, 30, 30)),
                Padding = new Thickness(2, 0, 2, 0),
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Bottom,
                TextAlignment = TextAlignment.Center
            };
            markerGrid.Children.Add(text);

            var marker = new GMapMarker(new PointLatLng(missile.Latitude, missile.Longitude))
            {
                Shape = markerGrid,
                Offset = new Point(-(markerSize-10) / 2, -(markerSize-10) / 2)
            };

            mapControl.Markers.Add(marker);
            _missileMarkers[missile.Id] = marker;
            _missilePrevPositions[missile.Id] = new PointLatLng(missile.Latitude, missile.Longitude);

            // 경로 Polyline 추가
            double angle = 0;
            if (_missilePrevPositions.TryGetValue(missile.Id, out var prev))
                angle = CalculateBearing(prev.Lat, prev.Lng, missile.Latitude, missile.Longitude);

            var tail = GetMissileTailPosition(missile, angle, markerSize - 10);
            var routePoints = new List<PointLatLng> { tail, new PointLatLng(missile.Latitude, missile.Longitude) };
            var route = new GMapRoute(routePoints);
            mapControl.Markers.Add(route);
            route.Shape = new System.Windows.Shapes.Path
            {
                Stroke = Brushes.Red,
                StrokeThickness = 4,
                StrokeDashArray = new DoubleCollection { 6, 4 },
                Opacity = 0.8
            };
            mapControl.RegenerateShape(route);

            _missileRoutes[missile.Id] = route;
            _missileRoutePoints[missile.Id] = routePoints;
        }


        private void RemoveMissileMarker(Missile missile)
        {
            if (_missileMarkers.TryGetValue(missile.Id, out var marker))
            {
                mapControl.Markers.Remove(marker);
                _missileMarkers.Remove(missile.Id);
            }

            if (_missileRoutes.TryGetValue(missile.Id, out var route))
            {
                mapControl.Markers.Remove(route);
                _missileRoutes.Remove(missile.Id);
                _missileRoutePoints.Remove(missile.Id);
            }
        }

        private void AttackDisplayPage_Loaded(object sender, RoutedEventArgs e)
        {
            //async로 시나리오 정보 가져오기
            getScenarioInfo(_scenarioId);
            // GMapControl에 포커스 강제 부여
            mapControl.Focus();
        }

        private void InitializeMap()
        {
            // config map
            mapControl.MapProvider = GMapProviders.GoogleMap;
            //getScenarioInfo()에서 Position 초기화함
            //mapControl.Position = new PointLatLng(37.5665, 126.9780);
            mapControl.MinZoom = 2;
            mapControl.MaxZoom = 18;
            mapControl.Zoom = 7;
            mapControl.ShowCenter = false;
            mapControl.CanDragMap = true;
            mapControl.MouseWheelZoomType = MouseWheelZoomType.MousePositionAndCenter;
            mapControl.DragButton = MouseButton.Left;
            //mapControl.MouseLeftButtonDown += mapControl_MouseLeftButtonDown;
            //mapControl.MouseLeftButtonUp += mapControl_MouseLeftButtonUp;

            mapControl.PreviewMouseWheel += (s, e) =>
            {
                if (e.Delta > 0)
                    mapControl.Zoom = Math.Min(mapControl.Zoom + 1, mapControl.MaxZoom);
                else
                    mapControl.Zoom = Math.Max(mapControl.Zoom - 1, mapControl.MinZoom);
                e.Handled = true;
            };
        }
        private async void getScenarioInfo(string scenarioId)
        {
            var coord = await _viewModel.GetScenarioInfoAsync(scenarioId);
            if (coord != null)
            {
                mapControl.Position = new PointLatLng(coord.latitude, coord.longitude);
                _viewModel.SetBatteryPos(coord.latitude, coord.longitude);
                // 항공기 마커 생성
                UpdateMarkers();
            }
            else
            {
                mapControl.Position = new PointLatLng(37.5665, 126.9780);
            }
        }


        // ㅇㅅㅇ
        private void UpdateMarkers()
        {
            //Debug.WriteLine($"[DEBUG] Aircraft count: {_viewModel.AircraftList.Aircrafts.Count}");
            //Debug.WriteLine($"[DEBUG] Missile  count: {_viewModel.MissileList.Missiles.Count}");
            Debug.WriteLine($"[DEBUG] BatteryPos set?: {_viewModel.BatteryPos != null}");

            /* ────────── 1) 포대 마커 (고정, 최초 1회) ────────── */
            if (_viewModel.BatteryPos is PointLatLng bp)
            {
                if (_batteryMarker == null)
                {
                    _batteryMarker = CreateBatteryImageMarker(bp, Brushes.Green, 50);
                    mapControl.Markers.Add(_batteryMarker);

                    // 300km, 130km 원 추가
                    var circle300 = CreateCircle(bp, 300, 120, Brushes.Red, null);
                    var circle130 = CreateCircle(bp, 130, 120, Brushes.ForestGreen, null);
                    mapControl.RegenerateShape(circle300);
                    mapControl.RegenerateShape(circle130);
                    mapControl.Markers.Add(circle300);
                    mapControl.Markers.Add(circle130);
                }
                else
                    _batteryMarker.Position = bp;   // (이동 없는 경우라 사실상 그대로)
            }
        }

        // 포대 이미지 마커로 변경
        private static GMapMarker CreateBatteryImageMarker(PointLatLng pos, Brush fill, double size)
        {
            // battery.png 이미지 경로 (절대경로 또는 상대경로로 수정)
            string imgPath = "pack://application:,,,/images/battery.png"; // 리소스 기반 경로

            var markerGrid = new Grid
            {
                Width = size,
                Height = size
            };

            var image = new Image
            {
                Width = size,
                Height = size,
                Source = new BitmapImage(new Uri(imgPath, UriKind.Absolute)),
                RenderTransformOrigin = new Point(0.5, 0.5),
                VerticalAlignment = VerticalAlignment.Center,
                HorizontalAlignment = HorizontalAlignment.Center
            };

            markerGrid.Children.Add(image);

            return new GMapMarker(pos)
            {
                Shape = markerGrid,
                Offset = new Point(-size / 2, -size / 2)
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

        // 항공기 방향각 계산 및 회전 함수
        private void UpdateAircraftMarkerDirection(AircraftWithIp aircraft)
        {
            if (_aircraftMarkers.TryGetValue(aircraft.Id, out var marker))
            {
                if (marker.Shape is Grid grid)
                {
                    var image = grid.Children.OfType<Image>().FirstOrDefault();
                    if (image != null)
                    {
                        double angle = CalculateBearing(aircraft.Latitude, aircraft.Longitude, _aircraftPrevPositions[aircraft.Id].Lat, _aircraftPrevPositions[aircraft.Id].Lng);
                        //image.RenderTransform = new RotateTransform(angle + 90, image.Width / 2, image.Height / 2);
                        image.RenderTransformOrigin = new Point(0.5, 0.5);
                        image.RenderTransform = new RotateTransform(angle + 90);
                    }
                }
            }
        }

        // 미사일 방향각 계산 및 회전
        private void UpdateMissileMarkerDirection(Missile missile)
        {
            if (_missileMarkers.TryGetValue(missile.Id, out var marker))
            {
                if (marker.Shape is Grid grid)
                {
                    var image = grid.Children.OfType<Image>().FirstOrDefault();
                    if (image != null)
                    {
                        double angle = CalculateBearing(_missilePrevPositions[missile.Id].Lat, _missilePrevPositions[missile.Id].Lng, missile.Latitude, missile.Longitude);
                        //image.RenderTransform = new RotateTransform(angle, image.Width / 2, image.Height / 2);
                        image.RenderTransformOrigin = new Point(0.5, 0.5);
                        image.RenderTransform = new RotateTransform(angle);
                        //Debug.WriteLine($"{missile.Id} 미사일 방향각 조정 : {angle}");
                    }
                }
            }
        }

        // bearing 계산 함수
        private double CalculateBearing(double lat1, double lon1, double lat2, double lon2)
        {
            double radLat1 = lat1 * Math.PI / 180.0;
            double radLat2 = lat2 * Math.PI / 180.0;
            double dLon = (lon2 - lon1) * Math.PI / 180.0;
            double y = Math.Sin(dLon) * Math.Cos(radLat2);
            double x = Math.Cos(radLat1) * Math.Sin(radLat2) -
                       Math.Sin(radLat1) * Math.Cos(radLat2) * Math.Cos(dLon);
            double bearing = Math.Atan2(y, x) * 180.0 / Math.PI;
            return (bearing + 360.0) % 360.0;
        }

    }
}