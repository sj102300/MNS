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
                MissileVisualState.EmergencyExplode => "images/explode.gif",  // 비폭과
                MissileVisualState.SelfExplode => "images/explode.gif",       // 자폭을 동일한 영상으로 처리
                MissileVisualState.Done => "images/empty.png",
                MissileVisualState.WeaponDataLink => "images/in_flight.gif",  // "images/weapon_datalink.gif", // 추가
                _ => "images/unknown.gif"
            };
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotImplementedException();
    }
}
