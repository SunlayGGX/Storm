using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Storm_LogViewer.Source.Log
{
    public enum LogLevelEnum
    {
        Debug,
        DebugWarning,
        DebugError,
        ScriptLogic,
        Comment,
        Warning,
        Error,
        Fatal,
        Always,

        // Special Level that expresses that the log is unknown.
        Unknown,

        // Special level that isn't really a level (not defined inside the real Storm application).
        // It is used to show a black line on the log viewer, telling us that the logs afterwards are from another sessions.
        NewSession
    }
}
