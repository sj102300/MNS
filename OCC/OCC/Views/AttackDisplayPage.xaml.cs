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
        //public AttackDisplayPage()
        //{
        //    InitializeComponent();
        //    DataContext = _viewModel;
        //    // Loaded 이벤트를 통해 NavigationService를 설정
        //    Loaded += AttackDisplayPage_Loaded;
        //    InitializeMap();
        //}
        public AttackDisplayPage(AttackViewModel viewModel)
        {
            InitializeComponent();
            _viewModel = viewModel;
            DataContext = _viewModel;
            // Loaded 이벤트를 통해 NavigationService를 설정
            Loaded += AttackDisplayPage_Loaded;
            InitializeMap();
            Debug.WriteLine($"AttackDisplayPage DataContext type: {DataContext?.GetType().Name}");
            Debug.WriteLine($"AttackDisplayPage에 전달된 ViewModel] HashCode: {viewModel.GetHashCode()}");


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
            getScenarioInfo();
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
        private async void getScenarioInfo()
        {
            var coord = await _viewModel.GetScenarioInfoAsync();
            if (coord != null)
                mapControl.Position = new PointLatLng(coord.latitude, coord.longitude);
            else
                mapControl.Position = new PointLatLng(37.5665, 126.9780);
        }

    }
}
