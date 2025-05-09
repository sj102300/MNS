using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace OCC.Models
{
    class Scenario
    {
        // 서버에서 생성(클라이언트에서는 null or 0 넣어서 보내기)
        [JsonPropertyName("scenario_id")]
        public string ScenarioId { get; set; }

        [JsonPropertyName("scenario_title")]
        public string ScenarioTitle { get; set; }

        [JsonPropertyName("aircraft_count")]
        public int AircraftCount { get; set; }

        [JsonPropertyName("aircraft")]
        public List<Aircraft> AircraftList { get; set; }

        [JsonPropertyName("battery_location")]
        public Coordinate BatteryLocation { get; set; }
    }
}
