using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Navigation;
using OCC.Commands;

namespace OCC.ViewModels
{
    public abstract class BaseViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler? PropertyChanged;

        // ICommand : MVVM 패턴에서 명령 패턴을 구현하기 위한 인터페이스
        public ICommand GoBackCommand { get; }

        public BaseViewModel()
        {
            GoBackCommand = new RelayCommand<object>(
                execute: _ => GoBack(),
                canExecute: _ => NavigationService?.CanGoBack == true
            );
        }
        public NavigationService? NavigationService { get; set; }

        public void GoBack()
        {
            // 이전 페이지로 돌아가기
            if (NavigationService?.CanGoBack == true)
            {
                NavigationService.GoBack();
            }
            else
            {
                // NavigationService가 null이거나 뒤로 갈 수 없는 경우 처리
                // 필요에 따라 사용자에게 알림을 표시하거나 로그를 남길 수 있음
                Debug.WriteLine("뒤로가기 동작을 수행할 수 없습니다.");
            }
        }

        public void GoInitPage()
        {
            // InitPage로 네비게이션
            //NavigationService.Navigate(new OCC.Views.InitPage(NavigationService));
            ////if (NavigationService != null)
            ////{
            ////    NavigationService.Navigate(new Uri("/Views/InitPage.xaml", UriKind.Relative));
            ////}
            ////else
            ////{
            ////    Debug.WriteLine("NavigationService가 설정되지 않았습니다.");
            ////}

            // AircraftLogWindow와 MissileLogWindow 닫기
            foreach (var win in System.Windows.Application.Current.Windows.OfType<System.Windows.Window>().ToList())
            {
                if (win.GetType().Name == "Aircraft Log" || win.GetType().Name == "Missile Log")
                {
                    win.Close();
                }
            }
        }

        // [CallerMemberName] : 이 함수를 호출한 대상에 대한 이름을 인자로 받음
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }
    }
}
