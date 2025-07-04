﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OCC.Utils
{
    public static class Network
    {
        public static readonly string SCN = "http://192.168.2.30:8080";
        public static readonly string MFR = "http://192.168.2.33:8080";
        //public static readonly string TCC = "http://192.168.2.66:8080";       //승엽
        //public static readonly string TCC = "http://192.168.2.65:8080";         //명준
        public static readonly string TCC = "http://192.168.2.64:8080";             //승주
        public static readonly string ATS = "http://192.168.2.55:8080";
        public static readonly string MSS = "http://192.168.2.22:8080";
        public static readonly string LCH = "http://192.168.2.11:8080";
        //public static readonly string TCCHost = "192.168.2.189";        //승엽
        //public static readonly string TCCHost = "192.168.2.188";        //명준
        public static readonly string TCCHost = "192.168.2.180";        //승주

        public static int TCCHostPort = 9999;
    }
}
