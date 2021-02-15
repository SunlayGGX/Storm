using Storm_CsHelper.Source.Config;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Storm_ScriptSender.Source.General.Config
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

        private UInt16 _port = Storm.NetworkConstants.k_defaultScriptSenderPort;
        public UInt16 Port
        {
            get => _port;
        }


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
                else if (ConfigManager.IsCommandLine(arg, "Port"))
                {
                    _port = UInt16.Parse(value);
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
            if (_port < 1500)
            {
                throw new Exception("Port below 1500 is dangerous! Aborting. Requested value was " + _port);
            }
        }

        #endregion
    }
}
