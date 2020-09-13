using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Storm_LogViewer.Source.General.Filterer
{
    class FiltererManager
    {
        #region Members

        #region Singleton

        private static FiltererManager s_instance = null;
        public static FiltererManager Instance
        {
            get
            {
                if (s_instance == null)
                {
                    s_instance = new FiltererManager();
                }

                return s_instance;
            }
        }

        #endregion

        #endregion
    }
}
