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
using Microsoft.Extensions.DependencyInjection;
using OCC.ViewModels;

namespace OCC.Views
{
    /// <summary>
    /// InitPage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class InitPage : Page
    {
        public InitPage(NavigationService navigationService)
        {
            InitializeComponent();

            // DI 컨테이너에서 NavigationService와 InitViewModel 가져오기
            if (navigationService == null)
            {
                throw new InvalidOperationException("NavigationService를 가져올 수 없습니다. InitPage가 NavigationWindow 또는 Frame 내에서 사용되고 있는지 확인하세요.");
            }
            var viewModel = new InitViewModel(navigationService);
            // NavigationService를 ViewModel에 전달
            DataContext = viewModel;
        }
    }

}
