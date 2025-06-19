using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace OCC.Models
{
    public class ScenarioListResponse
    {
        [JsonProperty("scenario_list")]
        public List<ScenarioGet> scenario_list { get; set; }
    }
}
