﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Models
{
    public enum MissileVisualState
    {
        Waiting = 0,
        Launching = 10,
        InFlight = 1,
        HitSuccess = 2,
        EmergencyExplode = 3,
        SelfExplode = 4,
        Done = 100 // empty.png
    }
}
