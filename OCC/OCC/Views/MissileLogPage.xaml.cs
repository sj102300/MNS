using System;
using System.Collections.Generic;
using System.Globalization;
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
using OCC.Models;
using OCC.ViewModels;
using WpfAnimatedGif;

namespace OCC.Views
{
    /// <summary>
    /// MissileLogPage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class MissileLogPage : Page
    {
        private AttackViewModel _viewModel;
        public MissileLogPage(AttackViewModel viewModel)
        {
            InitializeComponent();
            _viewModel = viewModel;
            DataContext = _viewModel;
        }

        private void Image_Loaded(object sender, RoutedEventArgs e)
        {
            if (sender is Image image && image.Tag is string gifPath)
            {
                try
                {
                    var uri = new Uri($"pack://application:,,,/{gifPath}");
                    var imageSource = new BitmapImage(uri);
                    ImageBehavior.SetAnimatedSource(image, imageSource);

                    ImageBehavior.AddAnimationCompletedHandler(image, (s, args) =>
                    {
                        if (image.DataContext is Missile missile)
                        {
                            if (missile.VisualState == MissileVisualState.Launching)
                                missile.VisualState = MissileVisualState.InFlight;
                            else if (missile.VisualState == MissileVisualState.PressingButton)
                                missile.VisualState = MissileVisualState.EmergencyExplode;
                        }
                    });
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"GIF 로딩 실패: {ex.Message}");
                }
            }
        }

    }
}
