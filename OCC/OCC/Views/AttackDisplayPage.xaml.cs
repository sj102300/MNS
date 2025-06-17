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
        private GMapMarker _batteryMarker;
        private string _scenarioId;

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

        //기존 항공기 마커 추가 메서드
        //private void AddAircraftMarker(AircraftWithIp aircraft)
        //{
        //    double markerSize = 20;
        //    var marker = new GMapMarker(new PointLatLng(aircraft.Latitude, aircraft.Longitude))
        //    {
        //        Shape = new Ellipse
        //        {
        //            Width = 16,
        //            Height = 16,
        //            Fill = Brushes.Red
        //        },
        //        Offset = new Point(-markerSize / 2, -markerSize / 2)
        //    };
        //    mapControl.Markers.Add(marker);
        //    _aircraftMarkers[aircraft.Id] = marker;
        //}

        //항공기 이미지 마커 변경 메서드
        private void AddImageAircraftMarker(AircraftWithIp aircraft)
        {
            double markerSize = 35;  // 아이콘 크기 조정

            // Grid로 이미지와 텍스트를 겹치게 배치
            var markerGrid = new Grid
            {
                Width = markerSize + 25,
                Height = markerSize + 5 // 텍스트 공간 확보
            };

            
            // 항공기 이미지
            var image = new Image
            {
                Width = markerSize,
                Height = markerSize,
                Source = new BitmapImage(new Uri(
                    @"C:\Users\user\source\repos\MNS\OCC\OCC\images\Aircraft.png",
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
        }

        //기존 IP 마커
        //private void AddIpMarker(ImpactPoint ip)
        //{
        //    var marker = new GMapMarker(new PointLatLng(ip.Latitude, ip.Longitude))
        //    {
        //        Shape = new Ellipse
        //        {
        //            Width = 12,
        //            Height = 12,
        //            Fill = Brushes.Purple
        //        },
        //        Offset = new Point(-5, -5)
        //    };
        //    mapControl.Markers.Add(marker);
        //    _ipMarkers[ip.Id] = marker;
        //}

        //이미지 IP 마커 변경 메서드
        private void AddImageIpMarker(ImpactPoint ip)
        {
            double markerSize = 20; // 아이콘 크기
            string imgPath = @"C:\Users\user\source\repos\MNS\OCC\OCC\images\impactPoint.png"; // 실제 경로로 수정

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
                    marker.Shape.Visibility = Visibility.Visible;
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

            // destroy.png로 새 마커 생성
            double markerSize;
            string destroyImgPath;
            if (missile_status == (uint)Missile.MissileStatus.HitSuccess)
            {
                markerSize = 150;
                destroyImgPath = @"C:\Users\user\source\repos\MNS\OCC\OCC\images\destroy.png";
            }
            else
            {
                markerSize = 100;
                destroyImgPath = @"C:\Users\user\source\repos\MNS\OCC\OCC\images\emergencyDestroy.png"; // 비상폭파 또는 자폭용 이미지
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
            
            if(missile_status != (uint)Missile.MissileStatus.HitSuccess){
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

            if (e.PropertyName == nameof(Missile.Status))
            {
                UpdateMissileMarkerVisibility(missile);

                // 상태가 격추완료(HitSuccess)면 마커를 destroy.png로 교체
                if (missile.Status == (uint)Missile.MissileStatus.HitSuccess ||
                    missile.Status == (uint)Missile.MissileStatus.SelfDestroy ||
                    missile.Status == (uint)Missile.MissileStatus.EmergencyDestroy)
                {
                    ReplaceMissileMarkerWithDestroyed(missile, missile.Status);
                } 
            }

            if (e.PropertyName == nameof(Missile.Latitude) || e.PropertyName == nameof(Missile.Longitude))
            {
                if (_missileMarkers.TryGetValue(missile.Id, out var marker))
                {
                    marker.Position = new PointLatLng(missile.Latitude, missile.Longitude);
                }
            }
        }

        //기존 미사일 마커 메서드
        //private void AddMissileMarker(Missile missile)
        //{
        //    var marker = new GMapMarker(new PointLatLng(missile.Latitude, missile.Longitude))
        //    {
        //        Shape = new Ellipse
        //        {
        //            Width = 16,
        //            Height = 16,
        //            Fill = Brushes.Blue
        //        },
        //        Offset = new Point(-5, -5)
        //    };

        //    mapControl.Markers.Add(marker);
        //    _missileMarkers[missile.Id] = marker;
        //}

        //미사일 이미지 마커 변경 메서드
        private void AddImageMissileMarker(Missile missile)
        {

            double markerSize = 35;  // 마커 이미지 크기 조정
            string imgPath = @"C:\Users\user\source\repos\MNS\OCC\OCC\images\missile.png";

            // Grid로 이미지와 텍스트를 겹치게 배치
            var markerGrid = new Grid
            {
                Width = markerSize + 25,
                Height = markerSize + 5 // 텍스트 공간 확보
            };

            // 미사일 이미지
            var image = new Image
            {
                Width = markerSize,
                Height = markerSize,
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
                Offset = new Point(-markerSize / 2, -markerSize / 2)
            };

            mapControl.Markers.Add(marker);
            _missileMarkers[missile.Id] = marker;
        }


        private void RemoveMissileMarker(Missile missile)
        {
            if (_missileMarkers.TryGetValue(missile.Id, out var marker))
            {
                mapControl.Markers.Remove(marker);
                _missileMarkers.Remove(missile.Id);
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
                    _batteryMarker = CreateBatteryImageMarker(bp, Brushes.Green, 40);
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

        //기존 포대 마커
        //private static GMapMarker CreateMarker(PointLatLng pos, Brush fill, double size)
        //{
        //    return new GMapMarker(pos)
        //    {
        //        Shape = new Ellipse
        //        {
        //            Width = size,
        //            Height = size,
        //            Fill = fill,
        //            Stroke = Brushes.Black,
        //            StrokeThickness = 1.5,
        //            Opacity = 0.9
        //        },
        //        Offset = new Point(-size / 2, -size / 2)
        //    };
        //}

        // 포대 이미지 마커로 변경
        private static GMapMarker CreateBatteryImageMarker(PointLatLng pos, Brush fill, double size)
        {
            // battery.png 이미지 경로 (절대경로 또는 상대경로로 수정)
            string imgPath = @"C:\Users\user\source\repos\MNS\OCC\OCC\images\battery.png";

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

    }
}
