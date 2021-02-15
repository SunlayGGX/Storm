using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Storm_LogViewer.Source.Script
{
    public class ScriptItem
    {
        private int _index;
        public int Index
        {
            get => _index;
            set => _index = value;
        }

        private string _scriptTextContent = string.Empty;
        public string ScriptTextContent
        {
            get => _scriptTextContent;
            set => _scriptTextContent = value;
        }
    }
}
