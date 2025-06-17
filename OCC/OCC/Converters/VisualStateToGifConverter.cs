using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Globalization;
using System.Windows.Data;
using OCC.Models;

namespace OCC.Converters
{
    public class VisualStateToGifConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var state = (OCC.Models.MissileVisualState)value;
            return state switch
            {
                MissileVisualState.Waiting => "images/waiting.gif",
                MissileVisualState.Launching => "images/launching.gif",
                MissileVisualState.InFlight => "images/in_flight.gif",
                MissileVisualState.HitSuccess => "images/hit_success.gif",
                MissileVisualState.PressingButton => "images/press_button.gif",
                MissileVisualState.EmergencyExplode => "images/emergency_explode.gif",
                MissileVisualState.SelfExplode => "images/self_explode.gif",
                _ => "images/unknown.gif"
            };
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) =>
            throw new NotImplementedException();
    }
}
