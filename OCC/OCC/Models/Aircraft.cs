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
        public string Id { get; set; }

        [JsonProperty("start_point")]
        public Coordinate StartPoint { get; set; }

        [JsonProperty("end_point")]
        public Coordinate EndPoint { get; set; }

        [JsonProperty("friend_or_foe")]
        public string FriendOrFoe { get; set; }
    }
}
