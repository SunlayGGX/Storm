using Storm_LogViewer.Source.Log;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

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
                    _onFilterCheckboxChanged?.Invoke();
                }
            }
        }

        private List<LogLevelFilterCheckboxValue> _logLevelsFilter;
        public List<LogLevelFilterCheckboxValue> LogLevelsFilter
        {
            get => _logLevelsFilter;
        }

        private List<ModuleFilterCheckboxValue> _moduleFilters = new List<ModuleFilterCheckboxValue>(12);
        public List<ModuleFilterCheckboxValue> ModuleFilters
        {
            get
            {
                lock (_moduleFilters)
                {
                    return _moduleFilters;
                }
            }
        }

        private bool _showEssentialOnly = false;
        public bool ShowEssentialOnly
        {
            get => _showEssentialOnly;
            set
            {
                if (_showEssentialOnly != value)
                {
                    _showEssentialOnly = value;
                    _onShowEssentialCheckboxChanged?.Invoke(value);
                }
            }
        }

        public bool _autoScrollEnabled = true;
        public bool AutoScrollEnabled
        {
            get => _autoScrollEnabled;
            set
            {
                if (_autoScrollEnabled != value)
                {
                    _autoScrollEnabled = value;
                    _onAutoScrollCheckboxChanged?.Invoke();
                }
            }
        }


        #region Events

        public delegate void OnFilterCheckboxChanged();
        public event OnFilterCheckboxChanged _onFilterCheckboxChanged;

        public delegate void OnModuleFilterAdded(List<ModuleFilterCheckboxValue> moduleList);
        public event OnModuleFilterAdded _onModuleFilterAdded;

        public delegate void OnShowEssentialCheckboxChanged(bool showEssential);
        public event OnShowEssentialCheckboxChanged _onShowEssentialCheckboxChanged;

        public delegate void OnAutoScrollCheckboxChanged();
        public event OnAutoScrollCheckboxChanged _onAutoScrollCheckboxChanged;

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

            _logLevelsFilter = new List<LogLevelFilterCheckboxValue>{
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Debug },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.DebugError },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Comment },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Warning },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Error },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Fatal },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Always },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Unknown }
            };
        }

        private void ValidateSettings()
        {
            if (_logFilePath != null && Path.GetExtension(_logFilePath).ToLower() != ".xml")
            {
                throw new Exception("log file to parse should be an xml file : current is " + _logFilePath);
            }
        }

        public void AddNewModuleFilters(List<string> modulesName)
        {
            if (modulesName.Count > 0)
            {
                List<ModuleFilterCheckboxValue> otherTmpSoThreadSafe;

                lock (_moduleFilters)
                {
                    foreach (string moduleName in modulesName)
                    {
                        ModuleFilterCheckboxValue newModuleFilter = new ModuleFilterCheckboxValue { _moduleName = moduleName };
                        LogReaderManager.Instance.ListenModuleFilterCheckedChangedEvent(newModuleFilter);
                        try
                        {
                            _moduleFilters.Add(newModuleFilter);
                        }
                        catch (System.Exception)
                        {
                            LogReaderManager.Instance.UnregisterFromModuleFilterCheckedChangedEvent(newModuleFilter);
                            throw;
                        }
                    }

                    otherTmpSoThreadSafe = _moduleFilters;
                }

                // Send the current state of the _moduleFilters... Not directly the reference of _moduleFilters...
                // It allows to unlock the call and only working with a snapshot of the module filter, in case this one is updated in another thread.
                // We won't have a data race (but we would work with deprecated data)... A tocttou can still happen but I don't care...
                _onModuleFilterAdded?.Invoke(otherTmpSoThreadSafe);
            }
        }

        #endregion
    }
}
