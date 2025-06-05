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
        private readonly Dictionary<string, GMapMarker> _missileMarkers = new();
        private GMapMarker _batteryMarker;
        private string _scenarioId;


        //public AttackDisplayPage()
        //{
        //    InitializeComponent();
        //    DataContext = _viewModel;
        //    // Loaded 이벤트를 통해 NavigationService를 설정
        //    Loaded += AttackDisplayPage_Loaded;
        //    InitializeMap();
        //}
        public AttackDisplayPage(AttackViewModel viewModel, string scenarioId)
        {
            InitializeComponent();
            _viewModel = viewModel;
            DataContext = _viewModel;
            _scenarioId = scenarioId;
            // Loaded 이벤트를 통해 NavigationService를 설정
            Loaded += AttackDisplayPage_Loaded;
            InitializeMap();

            //_viewModel.AircraftList.Aircrafts.CollectionChanged += (s, e) => UpdateMarkers();
            //_viewModel.MissileList.Missiles.CollectionChanged += (s, e) => UpdateMarkers();
            //_viewModel.PropertyChanged += (s, e) =>
            //{
            //    if (e.PropertyName == nameof(AttackViewModel.BatteryPos))
            //        UpdateMarkers();                     // BatteryPos 최초 세팅 시
            //};

            Debug.WriteLine($"AttackDisplayPage DataContext type: {DataContext?.GetType().Name}");
            Debug.WriteLine($"AttackDisplayPage에 전달된 ViewModel] HashCode: {viewModel.GetHashCode()}");
            Debug.WriteLine($"mapControl 타입 확인: {mapControl?.GetType().FullName}");

        }

        private void AttackDisplayPage_Loaded(object sender, RoutedEventArgs e)
        {
            //if (NavigationService != null)
            //{
            //    _viewModel._navigationService = NavigationService;
            //}
            //else if (Parent is Frame frame && frame.NavigationService != null)
            //{
            //    _viewModel.NavigationService = frame.NavigationService;
            //}

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
            mapControl.Zoom = 12;
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
                //UpdateMarkers();
            }
            else
            {
                mapControl.Position = new PointLatLng(37.5665, 126.9780);
                Debug.WriteLine($"[DEBUG]==============================================================");
                Debug.WriteLine($"[DEBUG]==============================================================");
                Debug.WriteLine($"[DEBUG]==============================================================");
                Debug.WriteLine($"[DEBUG]==============================================================");
            }
        }


        // ㅇㅅㅇ
        //private void UpdateMarkers()
        //{
        //    Debug.WriteLine($"[DEBUG] Aircraft count: {_viewModel.AircraftList.Aircrafts.Count}");
        //    Debug.WriteLine($"[DEBUG] Missile  count: {_viewModel.MissileList.Missiles.Count}");
        //    Debug.WriteLine($"[DEBUG] BatteryPos set?: {_viewModel.BatteryPos != null}");

        //    /* ────────── 1) 포대 마커 (고정, 최초 1회) ────────── */
        //    if (_viewModel.BatteryPos is PointLatLng bp)
        //    {
        //        if (_batteryMarker == null)
        //        {
        //            _batteryMarker = CreateMarker(bp, Brushes.Green, 16);
        //            mapControl.Markers.Add(_batteryMarker);
        //        }
        //        else
        //            _batteryMarker.Position = bp;   // (이동 없는 경우라 사실상 그대로)
        //    }

        //    /* ────────── 2) 항공기 동기화 ────────── */
        //    // 제거 대상
        //    var liveAircraftIds = _viewModel.AircraftList.Aircrafts.Select(a => a.AircraftId).ToHashSet();
        //    foreach (var id in _aircraftMarkers.Keys.Except(liveAircraftIds).ToList())
        //    {
        //        mapControl.Markers.Remove(_aircraftMarkers[id]);
        //        _aircraftMarkers.Remove(id);
        //    }
        //    // 추가/이동
        //    foreach (var ac in _viewModel.AircraftList.Aircrafts)
        //    {
        //        if (!_aircraftMarkers.TryGetValue(ac.AircraftId, out var mk))
        //        {
        //            mk = CreateMarker(default, Brushes.Red, 12);
        //            _aircraftMarkers[ac.AircraftId] = mk;
        //            mapControl.Markers.Add(mk);
        //        }
        //        mk.Position = new PointLatLng(ac.Latitude, ac.Longitude);

        //        Debug.WriteLine($"[A/C] {ac.AircraftId}  LAT:{ac.Latitude}  LON:{ac.Longitude}");
        //    }

        //    /* ────────── 3) 미사일 동기화 ────────── */
        //    var liveMissileIds = _viewModel.MissileList.Missiles.Select(m => m.MissileId).ToHashSet();
        //    foreach (var id in _missileMarkers.Keys.Except(liveMissileIds).ToList())
        //    {
        //        mapControl.Markers.Remove(_missileMarkers[id]);
        //        _missileMarkers.Remove(id);
        //    }
        //    foreach (var ms in _viewModel.MissileList.Missiles)
        //    {
        //        if (!_missileMarkers.TryGetValue(ms.MissileId, out var mk))
        //        {
        //            mk = CreateMarker(default, Brushes.Blue, 10);
        //            _missileMarkers[ms.MissileId] = mk;
        //            mapControl.Markers.Add(mk);
        //        }
        //        mk.Position = new PointLatLng(ms.Latitude, ms.Longitude);
        //    }
        //}

        private static GMapMarker CreateMarker(PointLatLng pos, Brush fill, double size)
        {
            return new GMapMarker(pos)
            {
                Shape = new Ellipse
                {
                    Width = size,
                    Height = size,
                    Fill = fill,
                    Stroke = Brushes.Black,
                    StrokeThickness = 1.5,
                    Opacity = 0.9
                },
                Offset = new Point(-size / 2, -size / 2)
            };
        }



    }
}
