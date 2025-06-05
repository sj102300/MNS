using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using OCC.Commands;
using System.Windows.Navigation;
using System.Windows;

namespace OCC.ViewModels
{
    public class InitViewModel : BaseViewModel
    {
        public ICommand NavigateToScenarioCreateCommand { get; }
        public ICommand NavigateToScenarioLoadCommand { get; }


        public InitViewModel(NavigationService navigationService)
        {
            NavigationService = navigationService ?? throw new ArgumentNullException(nameof(navigationService));

            NavigateToScenarioCreateCommand = new RelayCommand<object>(
                execute: _ => NavigateToScenarioCreate(),
                canExecute: _ => true
            );

            NavigateToScenarioLoadCommand = new RelayCommand<object>(
                execute: _ => NavigateToScenarioLoad(),
                canExecute: _ => true
            );
            
        }

        private void NavigateToScenarioCreate()
        {
            // ScenarioCreatePage.xaml로 이동
            Uri uri = new Uri("/Views/ScenarioCreatePage.xaml", UriKind.Relative);
            NavigationService.Navigate(uri);
        }

        private void NavigateToScenarioLoad()
        {
            // ScenarioLoadPage.xaml로 이동
            Uri uri = new Uri("/Views/ScenarioLoadPage.xaml", UriKind.Relative);
            NavigationService.Navigate(uri);
        }

        //private void NavigateToAttackDisplayPage()
        //{
        //    // AttackViewModel 하나 생성
        //    var attackViewModel = new OCC.ViewModels.AttackViewModel(NavigationService);
        //    // 현재 활성 창에 AttackDisplayPage를 띄우고 ViewModel 할당
        //    var currentWindow = Application.Current.Windows
        //                        .OfType<Window>()
        //                        .FirstOrDefault(w => w.IsActive);
        //    if (currentWindow != null)
        //    {
        //        var attackDisplayPage = new OCC.Views.AttackDisplayPage
        //        {
        //            DataContext = attackViewModel
        //        };
        //        currentWindow.Content = attackDisplayPage;
        //    }

        //    // AircraftLogPage를 Window에 담아서 띄우고 ViewModel 할당
        //    var aircraftLogPage = new OCC.Views.AircraftLogPage
        //    {
        //        DataContext = attackViewModel
        //    };
        //    var aircraftLogWindow = new Window
        //    {
        //        Title = "Aircraft Log",
        //        Width = 600,
        //        Height = 800,
        //        Content = aircraftLogPage
        //    };
        //    aircraftLogWindow.Show();

        //    // MissileLogPage를 Window에 담아서 띄우고 ViewModel 할당
        //    var missileLogPage = new OCC.Views.MissileLogPage
        //    {
        //        DataContext = attackViewModel
        //    };
        //    var missileLogWindow = new Window
        //    {
        //        Title = "Missile Log",
        //        Width = 600,
        //        Height = 800,
        //        Content = missileLogPage
        //    };
        //    missileLogWindow.Show();
        //}
    }
}
