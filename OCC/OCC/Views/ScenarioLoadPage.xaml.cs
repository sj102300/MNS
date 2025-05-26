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
    /// ScenarioLoadPage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class ScenarioLoadPage : Page
    {
        private ScenarioLoadViewModel _viewModel;
        public ScenarioLoadPage()
        {
            InitializeComponent();

            _viewModel = new ScenarioLoadViewModel();
            DataContext = _viewModel;

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
        }

        //private void ScenarioLoadPage_Loaded(object sender, RoutedEventArgs e)
        //{
        //    Debug.WriteLine("code behind execute");
        //    if (DataContext is ScenarioLoadViewModel vm && vm.LoadCommand.CanExecute(null))
        //    {
        //        vm.LoadCommand.Execute(null);
        //    }
        //}
        //private void ScenarioLoadPage_Loaded(object sender, RoutedEventArgs e)
        //{
        //    if (NavigationService != null)
        //    {
        //        _viewModel.NavigationService = NavigationService;
        //    }
        //    else if (Parent is Frame frame && frame.NavigationService != null)
        //    {
        //        _viewModel.NavigationService = frame.NavigationService;
        //    }
        //}
    }
}
