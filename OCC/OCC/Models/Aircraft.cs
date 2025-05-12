using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace OCC.Models
{
    class Aircraft
    {
        [JsonPropertyName("aircraft_id")]
        public string Id { get; set; }

        [JsonPropertyName("friend_or_foe")]
        public string FriendOrFoe { get; set; }

        [JsonPropertyName("start_point")]
        public Coordinate StartPoint { get; set; }

        [JsonPropertyName("end_point")]
        public Coordinate EndPoint { get; set; }
    }
}
