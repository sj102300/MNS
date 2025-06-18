using System;
using System.Collections.Generic;
using System.Diagnostics;
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
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using OCC.Models;
using OCC.Utils;
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
            //viewModel.VisualStatusChanged += (newState) =>
            //{
            //    string gifPath = GetGifPath(newState);
            //    LoadGif(gifPath);  // ImageBehavior.SetAnimatedSource 내부 처리
            //};

        }
        private void Image_TargetUpdated(object sender, DataTransferEventArgs e)
        {
            if (sender is Image image && image.Tag is string gifPath)
            {
                try
                {
                    var uri = new Uri($"pack://application:,,,/{gifPath}");
                    var imageSource = new BitmapImage(uri);
                    // 기본 설정: 반복 재생
                    ImageBehavior.SetRepeatBehavior(image, RepeatBehavior.Forever);

                    if (gifPath.Contains("launching") || gifPath.Contains("self_explode") || gifPath.Contains("hit_success") || gifPath.Contains("emergency_explode"))
                    {
                        // Launching일 경우 한 번만 재생
                        ImageBehavior.SetRepeatBehavior(image, new RepeatBehavior(1));
                    }

                    ImageBehavior.SetAnimatedSource(image, imageSource);

                    ImageBehavior.AddAnimationCompletedHandler(image, (s, args) =>
                    {
                        if (image.DataContext is Missile missile)
                        {
                            switch (missile.VisualState)
                            {
                                case MissileVisualState.Launching:
                                    missile.VisualState = MissileVisualState.InFlight;
                                    break;
                                case MissileVisualState.HitSuccess:
                                case MissileVisualState.EmergencyExplode:
                                case MissileVisualState.SelfExplode:
                                    missile.VisualState = MissileVisualState.Done;
                                    break;
                            }
                        }
                    });
                    Debug.Write("Image_TargetUpdated() called");
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"GIF 로딩 실패: {ex.Message}");
                }
            }
        }
        private void Image_Loaded(object sender, RoutedEventArgs e)
        {
            if (sender is Image image && image.Tag is string gifPath)
            {
                try
                {
                    //var uri = new Uri($"pack://application:,,,/{gifPath}");
                    //var imageSource = new BitmapImage(uri);
                    var imageSource = GifCache.Get(gifPath); // ← 캐싱된 BitmapImage 재사용
                    ImageBehavior.SetAnimatedSource(image, null);       // 이전 애니메이션 제거
                    ImageBehavior.SetAnimatedSource(image, imageSource);

                    ImageBehavior.AddAnimationCompletedHandler(image, (s, args) =>
                    {
                        if (image.DataContext is Missile missile)
                        {
                            switch (missile.VisualState)
                            {
                                case MissileVisualState.Launching:
                                    missile.VisualState = MissileVisualState.InFlight;
                                    break;
                                case MissileVisualState.HitSuccess:
                                case MissileVisualState.EmergencyExplode:
                                case MissileVisualState.SelfExplode:
                                    missile.VisualState = MissileVisualState.Done;
                                    break;
                            }
                        }
                    });
                    Debug.Write("Image_Loaded() called");
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"GIF 로딩 실패: {ex.Message}");
                }
            }
        }

    }
}
