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
                MissileVisualState.Waiting => "images/waiting.png?v={Guid.NewGuid()}",
                MissileVisualState.Launching => "images/launching.gif?v={Guid.NewGuid()}",
                MissileVisualState.InFlight => "images/in_flight.gif?v={Guid.NewGuid()}",
                MissileVisualState.HitSuccess => "images/hit_success.gif?v={Guid.NewGuid()}",
                MissileVisualState.EmergencyExplode => "images/explode.gif?v={Guid.NewGuid()}",  // 비폭과
                MissileVisualState.SelfExplode => "images/explode.gif?v={Guid.NewGuid()}",       // 자폭을 동일한 영상으로 처리
                MissileVisualState.Done => "images/empty.png?v={Guid.NewGuid()}",
                MissileVisualState.WeaponDataLink => "images/weapon_datalink.gif?v={Guid.NewGuid()}",  // 추가
                _ => "images/unknown.gif?v={Guid.NewGuid()}"
            };
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotImplementedException();
    }
}
