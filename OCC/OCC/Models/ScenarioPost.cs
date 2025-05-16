using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace OCC.Models
{
    public class ScenarioPost
    {
        // 서버에서 생성(클라이언트에서는 null or 0 넣어서 보내기)
        [JsonProperty("scenario_id")]
        public string scenario_id { get; set; }

        [JsonProperty("scenario_title")]
        public string scenario_title { get; set; }

        [JsonProperty("aircraft_count")]
        public int aircraft_count { get; set; }

        [JsonProperty("aircraft_list")]
        public List<Aircraft> aircraft_list { get; set; }

        [JsonProperty("battery_location")]
        public Coordinate battery_location { get; set; }
    }
}
