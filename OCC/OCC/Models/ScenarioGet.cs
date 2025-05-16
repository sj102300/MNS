using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace OCC.Models
{
    public class ScenarioGet
    {
        [JsonProperty("scenario_id")]
        public string scenario_id { get; set; }

        [JsonProperty("scenario_title")]
        public string scenario_title { get; set; }
    }
}
