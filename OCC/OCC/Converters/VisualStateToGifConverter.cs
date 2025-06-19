using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Globalization;
using System.Windows.Data;
using OCC.Models;
using System.Windows.Media.Imaging;
using System.IO;
using System.Diagnostics;

namespace OCC.Converters
{
    public class VisualStateToGifConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var state = (OCC.Models.MissileVisualState)value;
            return state switch
            {
                MissileVisualState.Waiting => "images/waiting.png",
                MissileVisualState.Launching => "images/launching.gif",
                MissileVisualState.InFlight => "images/in_flight.gif",
                MissileVisualState.HitSuccess => "images/hit_success.gif",
                MissileVisualState.EmergencyExplode => "images/emergency_explode.gif",
                MissileVisualState.SelfExplode => "images/self_explode.gif",
                MissileVisualState.Done => "images/empty.png",
                _ => "images/unknown.gif"
            };
        }

        //1.로딩 렉
        //    2. 상태전이 안됨
        //    0 -> 1 발사
        //    1 유지가 비행

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotImplementedException();
    }
}
