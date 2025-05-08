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
    /// InitPage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class InitPage : Page
    {
        public InitPage()
        {
            InitializeComponent();
        }

        private void ScenarioCreateBtn_Click(object sender, RoutedEventArgs e)
        {
            // SccenarioCreatePage.xaml로 이동
            Uri uri = new Uri("/Views/ScenarioCreatePage.xaml", UriKind.Relative);
            NavigationService.Navigate(uri);
        }
    }

}
