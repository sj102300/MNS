using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace OCC.Models
{
    public class Aircraft
    {
        [JsonProperty("aircraft_id")]
        public string aircraft_id { get; set; }

        [JsonProperty("start_point")]
        public Coordinate start_point { get; set; }

        [JsonProperty("end_point")]
        public Coordinate end_point { get; set; }

        [JsonProperty("friend_or_foe")]
        public string friend_or_foe { get; set; }
    }
}
