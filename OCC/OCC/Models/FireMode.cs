using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
namespace OCC.Models
{
    public class FireMode
    {
        public enum FireModeType { Auto = 0, Manual = 1 }

        public FireModeType _fireMode = FireModeType.Auto;
        public FireModeType Mode
        {
            get => _fireMode;
            set
            {
                if (_fireMode != value)
                {
                    _fireMode = value;
                    FireModeChanged?.Invoke(this, EventArgs.Empty);
                }
            }
        }

        public event EventHandler? FireModeChanged;
    }
}