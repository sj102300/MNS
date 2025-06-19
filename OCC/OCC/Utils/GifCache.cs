using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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

            var uri = new Uri($"pack://application:,,,/{relativePath}");
            var bitmap = new BitmapImage();
            bitmap.BeginInit();
            bitmap.UriSource = uri;
            bitmap.CacheOption = BitmapCacheOption.OnLoad; // 디스크 캐싱
            bitmap.CreateOptions = BitmapCreateOptions.IgnoreImageCache;
            bitmap.EndInit();
            bitmap.Freeze(); // UI Thread 외에서도 사용 가능

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
                "images/emergency_explode.gif",
                "images/self_explode.gif",
                "images/empty.png"
            };

            foreach (var path in paths)
            {
                _ = Get(path); // 강제로 로딩해서 캐시에 미리 저장
            }
        }

    }
}
