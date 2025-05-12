using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace OCC.ViewModels
{
    public class ScenarioCreateViewModel : BaseViewModel
    {
        public ICommand ScenarioSaveCommand { get; set; }

        public ScenarioCreateViewModel()
        {

        }
    }
}
