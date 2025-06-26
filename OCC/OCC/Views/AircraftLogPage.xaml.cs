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
using OCC.ViewModels;

namespace OCC.Views
{
    /// <summary>
    /// AircraftLogPage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class AircraftLogPage : Page
    {

        public AircraftLogPage(AttackViewModel viewModel)
        {
            InitializeComponent();
            //// NavigationService를 ViewModel에 전달
            DataContext = viewModel;
            Debug.WriteLine($"[AircraftLogPage에 전달된 ViewModel] HashCode: {viewModel.GetHashCode()}");
            Debug.WriteLine($"[AircraftLogPage] DataContext: {DataContext?.GetType().FullName}");

#if false
            Loaded += (_, __) =>
            {
                Debug.WriteLine($"[DEBUG] DataContext: {DataContext?.GetType().Name}");

                var changeModeText = ((AttackViewModel)DataContext).ChangeModeText;
                Debug.WriteLine($"[DEBUG] ChangeModeText 속성값 직접 호출: {changeModeText}");
            };
#endif
        }
    }
}
