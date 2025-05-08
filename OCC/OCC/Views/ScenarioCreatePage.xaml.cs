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

namespace OCC.Views
{
    /// <summary>
    /// ScenarioCreatePage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ScenarioCreatePage : Page
    {
        PointLatLng start;
        PointLatLng end;

        //marker
        GMapMarker currentMarker;

        //zones list
        List<GMapMarker> Circles = new List<GMapMarker>();

        public ScenarioCreatePage()
        {
            InitializeComponent();
            InitializeMap();
        }

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
        }
        private void mapControl_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            var point = e.GetPosition(mapControl);
            PointLatLng position = mapControl.FromLocalToLatLng((int)point.X, (int)point.Y);

            MessageBox.Show($"Lat : {position.Lat}, Long : {position.Lng}");

            GMapMarker marker = new GMapMarker(position)
            {
                Shape = new System.Windows.Shapes.Ellipse
                {
                    Width = 10,
                    Height = 10,
                    Stroke = Brushes.Black,
                    StrokeThickness = 2,
                    Fill = Brushes.Red
                }
            };
            mapControl.Markers.Add(marker);
        }
    }
}
