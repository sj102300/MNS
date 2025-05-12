using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using OCC.Commands;
using System.Windows.Navigation;

namespace OCC.ViewModels
{
    public class InitViewModel : BaseViewModel
    {
        public ICommand NavigateToScenarioCreateCommand { get; }

        private readonly NavigationService _navigationService;

        public InitViewModel(NavigationService navigationService)
        {
            _navigationService = navigationService ?? throw new ArgumentNullException(nameof(navigationService));

            NavigateToScenarioCreateCommand = new RelayCommand<object>(
                execute: _ => NavigateToScenarioCreate(),
                canExecute: _ => true
            );
        }

        private void NavigateToScenarioCreate()
        {
            // ScenarioCreatePage.xaml로 이동
            Uri uri = new Uri("/Views/ScenarioCreatePage.xaml", UriKind.Relative);
            _navigationService.Navigate(uri);
        }
    }
}
