using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Storm_LogViewer.Source.General
{
    class ConfigManager
    {
        #region Members

        private static ConfigManager s_instance = null;
        public static ConfigManager Instance
        {
            get
            {
                if (s_instance == null)
                {
                    s_instance = new ConfigManager();
                }

                return s_instance;
            }
        }

        #endregion
    }
}
