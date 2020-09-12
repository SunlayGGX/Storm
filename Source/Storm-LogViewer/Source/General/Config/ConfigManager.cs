using System;
using System.Collections.Generic;
using System.IO;

namespace Storm_LogViewer.Source.General.Config
{
    class ConfigManager
    {
        #region Members

        #region Statics

        private static ConfigManager s_instance = null;
        public static ConfigManager Instance
        {
            get => s_instance;
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

        private bool _filterStrictEquality = true;
        public bool FilterStrictEquality
        {
            get => _filterStrictEquality;
            set
            {
                if (_filterStrictEquality != value)
                {
                    _filterStrictEquality = value;
                    _onFilterCheckboxChanged?.Invoke(value);
                }
            }
        }


        #region Events

        public delegate void OnFilterCheckboxChanged(bool newValue);
        public event OnFilterCheckboxChanged _onFilterCheckboxChanged;

        #endregion

        #endregion

        #region Methods

        #region Constructor

        private ConfigManager(string[] args)
        {
            this.ParseCommandLines(args);
            this.ApplyDefaultSettingToRemainingConfig();
            this.ValidateSettings();
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

        public static void Create(string[] args)
        {
            if (s_instance == null)
            {
                s_instance = new ConfigManager(args);
            }
            else
            {
                throw new Exception("Cannot create twice a Singleton!");
            }
        }


        #endregion

        private void ParseCommandLines(string[] args)
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

        private void ValidateSettings()
        {
            if (_logFilePath != null && Path.GetExtension(_logFilePath).ToLower() != ".xml")
            {
                throw new Exception("log file to parse should be an xml file : current is " + _logFilePath);
            }
        }

        #endregion
    }
}
