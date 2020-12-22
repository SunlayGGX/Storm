using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Storm_LogViewer.Source.Log
{
    class PIDFilterCheckboxValue
    {
        public uint _pid;
        public uint PID
        {
            get => _pid;
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
