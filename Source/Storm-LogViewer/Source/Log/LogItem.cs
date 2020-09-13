using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Storm_LogViewer.Source.Log
{
    class LogItem
    {
        public string _timestamp = null;
        public string Timestamp
        {
            get => _timestamp;
            set => _timestamp = value;
        }

        public string _moduleName = null;
        public string ModuleName
        {
            get => _moduleName;
            set => _moduleName = value;
        }

        public LogLevelEnum _level = LogLevelEnum.Unknown;
        public LogLevelEnum LogLevel
        {
            get => _level;
            set => _level = value;
        }

        public string _codeLocation = null;
        public string CodeLocation
        {
            get => _codeLocation;
            set => _codeLocation = value;
        }

        public string _threadId = null;
        public string ThreadId
        {
            get => _threadId;
            set => _threadId = value;
        }

        public string _msg = null;
        public string Message
        {
            get => _msg;
            set => _msg = value;
        }
    }
}
