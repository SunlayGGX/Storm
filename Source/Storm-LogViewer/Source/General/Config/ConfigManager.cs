using System;
using System.Collections.Generic;

namespace Storm_LogViewer.Source.General.Config
{
    class ConfigManager
    {
        #region Members

        #region Statics

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

        private MacroConfig _macrosConfig = null;
        public MacroConfig MacrosConfig
        {
            get => _macrosConfig;
        }

        private string _logFilePath = null;
        public string LogFilePath
        {
            get => _logFilePath;
        }

        public bool HasLogFilePath
        {
            get => !string.IsNullOrEmpty(_logFilePath);
        }

        #endregion

        #region Methods

        #region Constructor

        public ConfigManager(List<string> args)
        {
            this.ParseCommandLines(args);
            this.ApplyDefaultSettingToRemainingConfig();
        }

        #endregion

        #region Statics

        private static string ParseCommandLineValue(string arg)
        {
            int posEqual = arg.IndexOf("=");
            if (posEqual != -1 && posEqual < arg.Length - 1)
            {
                return arg.Substring(posEqual + 1);
            }
            else
            {
                return null;
            }
        }

        private static bool IsCommandLine(string arg, string key)
        {
            return arg.StartsWith(key, StringComparison.InvariantCultureIgnoreCase);
        }


        #endregion

        private void ParseCommandLines(List<string> args)
        {
            foreach (string arg in args)
            {
                string value = ConfigManager.ParseCommandLineValue(arg);

                if (ConfigManager.IsCommandLine(arg, "MacroConfigFilePath"))
                {
                    _macrosConfig = new MacroConfig(value);
                }
                else if (ConfigManager.IsCommandLine(arg, "LogFilePath"))
                {
                    _logFilePath = value;
                }
            }
        }

        private void ApplyDefaultSettingToRemainingConfig()
        {
            if (_macrosConfig == null)
            {
                _macrosConfig = new MacroConfig();
            }
        }

        #endregion
    }
}
