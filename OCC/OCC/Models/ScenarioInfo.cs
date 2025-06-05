using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace OCC.Models
{
    internal class ScenarioInfo
    {
        [JsonProperty("battery_location")]

        public Coordinate coordinate { get; set; }
    }

}
