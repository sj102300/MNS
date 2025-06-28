using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;

namespace OCC.Utils
{
    public static class GifCache
    {
        private static readonly Dictionary<string, BitmapImage> _cache = new();

        public static BitmapImage Get(string relativePath)
        {
            if (_cache.TryGetValue(relativePath, out var cached))
                return cached;

            var uri = new Uri($"pack://application:,,,/{relativePath}", UriKind.Absolute);
            var bitmap = new BitmapImage();
            bitmap.BeginInit();
            bitmap.UriSource = uri;
            bitmap.CacheOption = BitmapCacheOption.OnLoad;  // 파일 다 읽고 메모리에 고정
            bitmap.CreateOptions = BitmapCreateOptions.PreservePixelFormat;
            bitmap.EndInit();
            bitmap.Freeze();  // 렌더링 성능 + 스레드 안전

            _cache[relativePath] = bitmap;
            return bitmap;
        }

        public static void PreloadAllGifs()
        {
            string[] paths = {
            "images/waiting.png",
            "images/launching.gif",
            "images/in_flight.gif",
            "images/hit_success.gif",
            "images/explode.gif",
            "images/empty.png",
            "images/weapon_datalink.gif"
        };

            foreach (var path in paths)
                _ = Get(path);  // 미리 로드해 두기
        }
    }
}
