using Storm_LogViewer.Source.General.Config;
using Storm_LogViewer.Source.Log;
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

        #region Statics

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

        private List<PIDFilterCheckboxValue> _pidsFilters = new List<PIDFilterCheckboxValue>(12);
        public List<PIDFilterCheckboxValue> PIDsFilters
        {
            get
            {
                lock (_pidsFilters)
                {
                    return _pidsFilters;
                }
            }
        }

        private string _lastFilter = string.Empty;

        #region Events

        public delegate void OnModuleFilterAdded(List<ModuleFilterCheckboxValue> moduleList);
        public event OnModuleFilterAdded _onModuleFilterAdded;

        public delegate void OnPIDFilterAdded(List<PIDFilterCheckboxValue> pidsList);
        public event OnPIDFilterAdded _onPIDFilterAdded;

        #endregion

        #endregion

        #region Methods

        #region Constructor

        public FiltererManager()
        {
            _logLevelsFilter = new List<LogLevelFilterCheckboxValue>{
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Debug },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.DebugWarning },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.DebugError },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.ScriptLogic },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Comment },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Warning },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Error },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Fatal },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Always },
                new LogLevelFilterCheckboxValue{ _level = LogLevelEnum.Unknown }
            };
        }

        #endregion

        private bool ApplyLogLevelFilter(ref IEnumerable<LogItem> logItemsEnumerable)
        {
            List<LogLevelFilterCheckboxValue> logLevelFilters = _logLevelsFilter.Where(logLevelFilter => logLevelFilter.Checked).ToList();
            if (logLevelFilters.Count != _logLevelsFilter.Count)
            {
                // Add the NewSession since it isn't really a log to be filtered (this is a separator).
                logLevelFilters.Add(new LogLevelFilterCheckboxValue { _level = LogLevelEnum.NewSession });
                logItemsEnumerable = logItemsEnumerable.Where(item => logLevelFilters.Any(lvFilter => lvFilter.LogLevel == item.LogLevel));

                return true;
            }

            return false;
        }

        private bool ApplyModuleFilter(ref IEnumerable<LogItem> logItemsEnumerable)
        {
            List<ModuleFilterCheckboxValue> modulesFilters = _moduleFilters.Where(moduleFilter => moduleFilter.Checked).ToList();
            if (modulesFilters.Count != _moduleFilters.Count)
            {
                logItemsEnumerable = logItemsEnumerable.Where(item => item.LogLevel == LogLevelEnum.NewSession || modulesFilters.Any(modFilter => modFilter.ModuleName == item.ModuleName));

                return true;
            }

            return false;
        }

        private bool ApplyPIDFilter(ref IEnumerable<LogItem> logItemsEnumerable)
        {
            List<PIDFilterCheckboxValue> pidsFilters = _pidsFilters.Where(pidFilter => pidFilter.Checked).ToList();
            if (pidsFilters.Count != _pidsFilters.Count)
            {
                logItemsEnumerable = logItemsEnumerable.Where(item => item.LogLevel == LogLevelEnum.NewSession || pidsFilters.Any(pidFilter => pidFilter.PID == item.PID));

                return true;
            }

            return false;
        }

        private bool ApplyFilterText(ref IEnumerable<LogItem> logItemsEnumerable, string newFilterText)
        {
            if (!string.IsNullOrEmpty(newFilterText))
            {
                ConfigManager configMgr = ConfigManager.Instance;
                if (configMgr.FilterStrictEquality)
                {
                    logItemsEnumerable = logItemsEnumerable.Where(item => item.LogLevel == LogLevelEnum.NewSession || item.Message.Contains(newFilterText)).ToList();
                }
                else
                {
                    string[] split = newFilterText.Split(' ');
                    logItemsEnumerable = logItemsEnumerable.Where(item => split.Any(splitFilter => item.LogLevel == LogLevelEnum.NewSession || item.Message.Contains(splitFilter)));
                }

                return true;
            }

            return false;
        }

        public List<LogItem> ApplyFilters(List<LogItem> logItems, string newFilterText)
        {
            string finalFilter = newFilterText ?? string.Empty;
            if (!ConfigManager.Instance.FilterStrictEquality)
            {
                finalFilter = finalFilter.Trim();
            }

            lock(_lastFilter)
            {
                _lastFilter = finalFilter;
            }

            bool hasAppliedAtLeastOneFilter = false;

            IEnumerable<LogItem> logFilterIEnum = logItems;
            hasAppliedAtLeastOneFilter |= this.ApplyLogLevelFilter(ref logFilterIEnum);
            hasAppliedAtLeastOneFilter |= this.ApplyPIDFilter(ref logFilterIEnum);
            hasAppliedAtLeastOneFilter |= this.ApplyModuleFilter(ref logFilterIEnum);
            hasAppliedAtLeastOneFilter |= this.ApplyFilterText(ref logFilterIEnum, finalFilter);

            if (hasAppliedAtLeastOneFilter)
            {
                return logFilterIEnum.ToList();
            }
            else
            {
                return logItems;
            }
        }

        public List<LogItem> ApplyFilters(List<LogItem> logItems)
        {
            string finalFilter;
            lock (_lastFilter)
            {
                finalFilter = _lastFilter;
            }

            return this.ApplyFilters(logItems, finalFilter);
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

        public void AddNewPIDFilters(List<uint> newPids)
        {
            if (newPids.Count > 0)
            {
                List<PIDFilterCheckboxValue> otherTmpSoThreadSafe;

                lock (_pidsFilters)
                {
                    foreach (uint pid in newPids)
                    {
                        PIDFilterCheckboxValue newPIDFilter = new PIDFilterCheckboxValue { _pid = pid };
                        LogReaderManager.Instance.ListenPIDFilterCheckedChangedEvent(newPIDFilter);
                        try
                        {
                            _pidsFilters.Add(newPIDFilter);
                        }
                        catch (System.Exception)
                        {
                            LogReaderManager.Instance.UnregisterFromPIDFilterCheckedChangedEvent(newPIDFilter);
                            throw;
                        }
                    }

                    otherTmpSoThreadSafe = _pidsFilters;
                }

                // Send the current state of the _pidsFilters... Not directly the reference of _pidsFilters...
                // It allows to unlock the call and only working with a snapshot of the pid filter, in case this one is updated in another thread.
                // We won't have a data race (but we would work with deprecated data)... A tocttou can still happen but I don't care...
                _onPIDFilterAdded?.Invoke(otherTmpSoThreadSafe);
            }
        }

        #endregion
    }
}
