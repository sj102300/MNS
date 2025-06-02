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

namespace OCC.Views
{
    /// <summary>
    /// AttackDisplayPage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class AttackDisplayPage : Page
    {
        private Point _mouseDownPoint;
        private DateTime _mouseDownTime;
        private const double CLICK_THRESHOLD = 5.0; // 픽셀 거리
        private const int CLICK_TIME_MS = 300; // 클릭 시간 제한 (밀리초)

        private AttackDisplayViewModel _viewModel;

        public AttackDisplayPage()
        {
            InitializeComponent();
            _viewModel = new AttackDisplayViewModel();
            DataContext = _viewModel;
            InitializeMap();

            // Loaded 이벤트를 통해 NavigationService를 설정
            Loaded += ScenarioLoadPage_Loaded;
        }

        private void ScenarioLoadPage_Loaded(object sender, RoutedEventArgs e)
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
            mapControl.Zoom = 12;
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

        private void mapControl_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            
        }

        private void mapControl_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
 
            // 이벤트 전파 차단 -> 다른 컨트롤에 이벤트가 전달되지 않도록 함(한번 클릭해도 두번 눌리는 현상 방지)
            e.Handled = true;
        }
    }
}
