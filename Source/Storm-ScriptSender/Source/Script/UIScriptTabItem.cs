using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Storm_ScriptSender.Source.Script
{
    public class UIScriptTabItem
    {
        private string _title = string.Empty;
        public string Title
        {
            get => _title;
            set => _title = value;
        }

        private ObservableCollection<ScriptItem> _items = new ObservableCollection<ScriptItem>();
        public ObservableCollection<ScriptItem> Items
        {
            get => _items;
            set => _items = value;
        }
    }
}
