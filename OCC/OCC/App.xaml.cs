using System.Configuration;
using System.Data;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Navigation;
using Microsoft.Extensions.DependencyInjection;
using OCC.Utils;
using OCC.ViewModels;
using Wpf.Ui.Appearance;

namespace OCC;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    public IServiceProvider ServiceProvider { get; private set; }

    protected override void OnStartup(StartupEventArgs e)
    {
        var services = new ServiceCollection();

        // NavigationService를 싱글톤으로 등록
        services.AddSingleton<NavigationService>();

        // ViewModel 등록
        services.AddTransient<InitViewModel>();

        ServiceProvider = services.BuildServiceProvider();

        base.OnStartup(e);

        //GifCache.PreloadAllGifs();  // 로딩 렉 심해서 GIF 캐싱
    }
}

