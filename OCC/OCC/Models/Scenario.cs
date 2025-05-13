using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace OCC.Models
{
    public class Scenario
    {
        // 서버에서 생성(클라이언트에서는 null or 0 넣어서 보내기)
        [JsonProperty("scenario_id")]
        public string ScenarioId { get; set; }

        [JsonProperty("scenario_title")]
        public string ScenarioTitle { get; set; }

        [JsonProperty("aircraft_count")]
        public int AircraftCount { get; set; }

        [JsonProperty("aircraft")]
        public List<Aircraft> AircraftList { get; set; }

        [JsonProperty("battery_location")]
        public Coordinate BatteryLocation { get; set; }
    }
}
