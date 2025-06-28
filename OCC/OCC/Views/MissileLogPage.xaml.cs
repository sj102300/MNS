using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Security.Policy;
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
        }

        // 상태 전이 관리
        private void OnGifCompleted(object sender, RoutedEventArgs e)
        {
            //Debug.WriteLine($"[OnGifCompleted 호출] 1회 실행 및 상태전이");

            //if (sender is Image image && image.DataContext is Missile missile)
            if (sender is Image image && image.DataContext is AttackViewModel vm && vm.SelectedMissile is Missile missile)
            {
                //Debug.WriteLine($"[조건문 진입] Missile={missile.Id}, VisualState={missile.VisualState}");
                
                if (missile.VisualState == MissileVisualState.Done)
                    return;

                switch (missile.VisualState)
                {
                    case MissileVisualState.Launching:
                    case MissileVisualState.WeaponDataLink:
                        missile.VisualState = MissileVisualState.InFlight;
                        break;

                    case MissileVisualState.HitSuccess:
                    case MissileVisualState.EmergencyExplode:
                    case MissileVisualState.SelfExplode:
                        missile.VisualState = MissileVisualState.Done;
                        break;
                }
            }
        }

        // 핸들러 분리
        private void GIF_Updated(object sender, DataTransferEventArgs e)
        {
            if (sender is Image image && image.Tag is string gifPath)
            {
                try
                {
                    var imageSource = GifCache.Get(gifPath);

                    if (gifPath.Contains("launching") || gifPath.Contains("explode") || gifPath.Contains("hit_success") || gifPath.Contains("weapon_datalink"))
                    {
                        ImageBehavior.SetRepeatBehavior(image, new RepeatBehavior(1));
                    }
                    else
                    {
                        ImageBehavior.SetRepeatBehavior(image, RepeatBehavior.Forever);
                    }

                    ImageBehavior.SetAnimatedSource(image, null);
                    ImageBehavior.SetAnimatedSource(image, imageSource);

                    //// 기존 핸들러 제거
                    ImageBehavior.RemoveAnimationCompletedHandler(image, OnGifCompleted);

                    //// 새 핸들러 등록
                    ImageBehavior.AddAnimationCompletedHandler(image, OnGifCompleted);

                    RenderOptions.SetBitmapScalingMode(image, BitmapScalingMode.HighQuality);
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"GIF 로딩 실패: {ex.Message}");
                }
            }
        }
    }
}
