using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Storm_LogViewer.Source.Log
{
    class ModuleFilterCheckboxValue
    {
        public string _moduleName = null;
        public string ModuleName
        {
            get => _moduleName;
        }

        public bool _checked = true;
        public bool Checked
        {
            get => _checked;
            set
            {
                if (_checked != value)
                {
                    _checked = value;
                    _onCheckedStateChanged?.Invoke();
                }
            }
        }

        public delegate void OnCheckedStateChanged();
        public event OnCheckedStateChanged _onCheckedStateChanged;
    }
}
