using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Models
{
    internal class FireMode
    {
        // 발사 모드 enum
        public enum FireModeType { Auto = 0, Manual }

        private FireModeType _fireMode = FireModeType.Auto;
        public FireModeType Mode
        {
            get => _fireMode;
            set
            {
                if (_fireMode != value)
                {
                    _fireMode = value;
                    OnFireModeChanged();
                }
            }
        }
        public event Action? FireModeChanged;
        protected virtual void OnFireModeChanged()
        {
            FireModeChanged?.Invoke();
        }
    }
}
