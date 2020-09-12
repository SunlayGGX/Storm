using Storm_LogViewer.Source.Converters;
using Storm_LogViewer.Source.General.Config;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Xml;

namespace Storm_LogViewer.Source.Log
{
    class LogReaderManager
    {
        #region Members

        private System.DateTime _lastLogFileWriteTime = DateTime.MinValue;
        private System.DateTime _lastLogFileCreationTime = DateTime.MinValue;

        private bool _isRunning = true;
        public bool IsRunning
        {
            get => _isRunning;
        }

        private List<LogItem> _logItems = new List<LogItem>(0);
        public List<LogItem> LogItems
        {
            get => _logItems;
        }

        private List<LogItem> _displayedLogItems;
        public List<LogItem> DisplayedLogItems
        {
            get => _displayedLogItems;
        }

        Thread _parserWatcherThread = null;

        long _lastStreamPos = 0;

        string _lastFilter = string.Empty;

        List<string> _moduleList = new List<string>(12);

        bool _shouldClearLogs = false;

        #endregion

        #region Events

        public delegate void OnDisplayedLogItemsCollectionChanged(List<LogItem> displayedLogItems, int maxLogCount);
        public event OnDisplayedLogItemsCollectionChanged _onDisplayedLogItemsCollectionChanged;

        #endregion


        #region Singleton

        static LogReaderManager s_instance = null;
        public static LogReaderManager Instance
        {
            get
            {
                if (s_instance == null)
                {
                    s_instance = new LogReaderManager();
                }

                return s_instance;
            }
        }

        #endregion

        #region Methods

        #region Constructor

        private LogReaderManager()
        {
            _displayedLogItems = _logItems;
        }

        #endregion

        public void Init()
        {
            ConfigManager.Instance._onFilterCheckboxChanged += this.OnFiltersChanged;
            foreach (LogLevelFilterCheckboxValue checkboxValue in ConfigManager.Instance.LogLevelsFilter)
            {
                checkboxValue._onCheckedStateChanged += this.OnFiltersChanged;
            }

            this.TryRunParsingOnce();

            _parserWatcherThread = new Thread(() => this.Run());
            _parserWatcherThread.Start();
        }

        private static bool HandleAttributeRead<Converter, ValueType>(XmlNode xmlNode, string attributeTag, ref ValueType value, Converter converter) where Converter : System.Delegate
        {
            var attrib = xmlNode.Attributes[attributeTag];
            if (attrib != null)
            {
                value = (ValueType)converter.DynamicInvoke(attrib.Value);
                return true;
            }

            return false;
        }

        private void ParseLogFile(FileInfo logFileInfo)
        {
            XmlDocument doc = null;

            int watchdog = 0;
            do
            {
                if (!_isRunning)
                {
                    return;
                }

                try
                {
                    logFileInfo.Refresh();

                    // If the Storm logger is set to override the file, then the last stream pos is invalid.
                    DateTime logFileCreationTime = logFileInfo.CreationTime;
                    if (_lastLogFileCreationTime < logFileCreationTime)
                    {
                        _lastLogFileCreationTime = logFileCreationTime;
                        _lastStreamPos = 0;
                    }

                    int initialFileSize = (int)logFileInfo.Length;
                    using (FileStream filestream = new FileStream(logFileInfo.FullName, FileMode.Open, FileAccess.Read, FileShare.ReadWrite, initialFileSize, FileOptions.RandomAccess))
                    {
                        filestream.Position = _lastStreamPos;

                        string content = "<tmp>\n";
                        using (StreamReader reader = new StreamReader(filestream))
                        {
                            logFileInfo.Refresh();
                            _lastLogFileWriteTime = logFileInfo.LastWriteTime;

                            content += reader.ReadToEnd();

                            _lastStreamPos = filestream.Position;
                        }

                        doc = new XmlDocument();
                        doc.LoadXml(content + "</tmp>");
                    }
                }
                catch (System.Exception)
                {
                    doc = null;
                }

                if (doc == null)
                {
                    Thread.Sleep(100);
                    if (!_isRunning)
                    {
                        return;
                    }
                }

            } while (doc == null && ++watchdog < 10);

            if (watchdog == 10 && doc == null)
            {
                return;
            }

            int preCollectionCount = _logItems.Count;

            List<string> newModuleAddedThisFrame = new List<string>(12);

            foreach (XmlNode logItemXml in doc.ChildNodes[0].ChildNodes)
            {
                LogItem item = new LogItem();

                if (logItemXml.Name != "separator")
                {
                    if (
                        logItemXml.InnerText != null &&
                        HandleAttributeRead<Func<string, LogLevelEnum>, LogLevelEnum>(logItemXml, "logLevel", ref item._level, (string val) => LogLevelToStringConverter.FromString(val)) &&
                        HandleAttributeRead<Func<string, string>, string>(logItemXml, "module", ref item._moduleName, (string val) => val) &&
                        HandleAttributeRead<Func<string, string>, string>(logItemXml, "timestamp", ref item._timestamp, (string val) => val) &&
                        HandleAttributeRead<Func<string, string>, string>(logItemXml, "codeLocation", ref item._codeLocation, (string val) => val) &&
                        HandleAttributeRead<Func<string, string>, string>(logItemXml, "thread", ref item._threadId, (string val) => val)
                        )
                    {
                        item.Message = logItemXml.InnerText;
                        if (item.Message != null)
                        {
                            this.AddLogItem(item, ref newModuleAddedThisFrame);
                        }
                    }
                }
                else
                {
                    this.AddLogItem(item, ref newModuleAddedThisFrame);
                }
            }

            ConfigManager.Instance.AddNewModuleFilters(newModuleAddedThisFrame);

            if (preCollectionCount != _logItems.Count && _isRunning)
            {
                lock(_lastFilter)
                {
                    this.ApplyFilterInternalNoCheck(_lastFilter);
                }
                this.NotifyLogItemsCollectionChanged();
            }
        }

        public void NotifyLogItemsCollectionChanged()
        {
            _onDisplayedLogItemsCollectionChanged?.Invoke(_displayedLogItems, _logItems.Count);
        }

        private void Run()
        {
            while (_isRunning)
            {
                Thread.Sleep(500);

                if (_isRunning)
                {
                    DoClearLogsIfNeeded();
                    this.TryRunParsingOnce();
                }
            }
        }

        private void TryRunParsingOnce()
        {
            string logFilePath;
            if (ConfigManager.Instance.HasLogFilePath)
            {
                logFilePath = ConfigManager.Instance.LogFilePath;
            }
            else
            {
                logFilePath = RetrieveLastLogFile();
            }

            if (_isRunning && logFilePath != null)
            {
                FileInfo fileInfo = new FileInfo(logFilePath);
                if (fileInfo.LastWriteTime > _lastLogFileWriteTime)
                {
                    this.ParseLogFile(fileInfo);
                }
            }
        }

        public void Shutdown()
        {
            _isRunning = false;
            _parserWatcherThread.Join();

            ConfigManager.Instance._onFilterCheckboxChanged -= this.OnFiltersChanged;
            foreach (LogLevelFilterCheckboxValue checkboxValue in ConfigManager.Instance.LogLevelsFilter)
            {
                checkboxValue._onCheckedStateChanged -= this.OnFiltersChanged;
            }
            foreach (ModuleFilterCheckboxValue checkboxValue in ConfigManager.Instance.ModuleFilters)
            {
                checkboxValue._onCheckedStateChanged -= this.OnFiltersChanged;
            }
        }

        public string RetrieveLastLogFile()
        {
            FileInfo lastFile = null;
            DateTime lastDateTime = DateTime.MinValue;

            const int k_watchdogLastCount = 10;

            string logFileFolder = Path.Combine(ConfigManager.Instance.MacrosConfig.GetMacroEndValue("StormIntermediate"), "Logs");
            foreach (FileInfo file in new DirectoryInfo(logFileFolder).EnumerateFiles())
            {
                if (file.Extension.ToLower() == ".xml")
                {
                    int watchdog = 0;
                    do
                    {
                        try
                        {
                            if (lastFile == null || file.LastWriteTime > lastDateTime)
                            {
                                lastFile = file;
                                lastDateTime = file.LastWriteTime;
                                watchdog = k_watchdogLastCount;
                            }
                        }
                        catch (System.Exception)
                        {
                            Thread.Sleep(100);

                            if (!_isRunning)
                            {
                                return null;
                            }

                            file.Refresh();
                        }

                    } while (++watchdog < k_watchdogLastCount);
                }
            }

            if (lastFile != null)
            {
                lastFile.Refresh();
                return lastFile.Exists ? lastFile.FullName : null;
            }
            else
            {
                return null;
            }
        }

        private void ApplyFilterInternalNoCheck(string filter)
        {
            List<LogLevelFilterCheckboxValue> logLevelFilters = ConfigManager.Instance.LogLevelsFilter.Where(logLevelFilter => logLevelFilter.Checked).ToList();
            bool hasNotLogLevelFilter = logLevelFilters.Count == ConfigManager.Instance.LogLevelsFilter.Count;
            List<ModuleFilterCheckboxValue> modulesFilters = ConfigManager.Instance.ModuleFilters.Where(moduleFilter => moduleFilter.Checked).ToList();
            bool hasNotModuleFilter = modulesFilters.Count == ConfigManager.Instance.ModuleFilters.Count;

            _lastFilter = filter ?? string.Empty;
            if (_lastFilter == string.Empty)
            {
                if (hasNotLogLevelFilter)
                {
                    if (hasNotModuleFilter)
                    {
                        _displayedLogItems = _logItems;
                    }
                    else
                    {
                        _displayedLogItems = _logItems.Where(item => modulesFilters.Any(modFilter => modFilter.ModuleName == item.ModuleName)).ToList();
                    }
                }
                else
                {
                    if (hasNotModuleFilter)
                    {
                        _displayedLogItems = _logItems.Where(item => logLevelFilters.Any(lvFilter => lvFilter.LogLevel == item.LogLevel)).ToList();
                    }
                    else
                    {
                        _displayedLogItems = _logItems
                            .Where(item => logLevelFilters.Any(lvFilter => lvFilter.LogLevel == item.LogLevel))
                            .Where(item => modulesFilters.Any(modFilter => modFilter.ModuleName == item.ModuleName)).ToList();
                    }
                }
            }
            else
            {
                if (ConfigManager.Instance.FilterStrictEquality)
                {
                    if (hasNotLogLevelFilter)
                    {
                        if (hasNotModuleFilter)
                        {
                            _displayedLogItems = _logItems
                                .Where(item => logLevelFilters.Any(lvFilter => lvFilter.LogLevel == item.LogLevel))
                                .Where(item => item.Message.Contains(filter)).ToList();
                        }
                        else
                        {
                            _displayedLogItems = _logItems
                                .Where(item => logLevelFilters.Any(lvFilter => lvFilter.LogLevel == item.LogLevel))
                                .Where(item => modulesFilters.Any(modFilter => modFilter.ModuleName == item.ModuleName))
                                .Where(item => item.Message.Contains(filter)).ToList();
                        }
                    }
                    else
                    {
                        if (hasNotModuleFilter)
                        {
                            _displayedLogItems = _logItems.Where(item => item.Message.Contains(filter)).ToList();
                        }
                        else
                        {
                            _displayedLogItems = _logItems
                                .Where(item => modulesFilters.Any(modFilter => modFilter.ModuleName == item.ModuleName))
                                .Where(item => item.Message.Contains(filter)).ToList();
                        }
                    }
                }
                else
                {
                    string[] split = filter.Split(' ');
                    if (hasNotLogLevelFilter)
                    {
                        if (hasNotModuleFilter)
                        {
                            _displayedLogItems = _logItems
                                .Where(item => logLevelFilters.Any(lvFilter => lvFilter.LogLevel == item.LogLevel))
                                .Where(item => split.Any(splitFilter => item.Message.Contains(splitFilter))).ToList();
                        }
                        else
                        {
                            _displayedLogItems = _logItems
                                .Where(item => logLevelFilters.Any(lvFilter => lvFilter.LogLevel == item.LogLevel))
                                .Where(item => modulesFilters.Any(modFilter => modFilter.ModuleName == item.ModuleName))
                                .Where(item => split.Any(splitFilter => item.Message.Contains(splitFilter))).ToList();
                        }
                    }
                    else
                    {
                        if (hasNotModuleFilter)
                        {
                            _displayedLogItems = _logItems.Where(item => split.Any(splitFilter => item.Message.Contains(splitFilter))).ToList();
                        }
                        else
                        {
                            _displayedLogItems = _logItems
                                .Where(item => modulesFilters.Any(modFilter => modFilter.ModuleName == item.ModuleName))
                                .Where(item => split.Any(splitFilter => item.Message.Contains(splitFilter))).ToList();
                        }
                    }
                }
            }
        }

        private void ApplyFilterInternal(string filter)
        {
            if (filter == _lastFilter)
            {
                return;
            }

            this.ApplyFilterInternalNoCheck(filter);
        }

        public void ApplyFilter(string filter)
        {
            lock(_lastFilter)
            {
                this.ApplyFilterInternal(filter);
            }
            this.NotifyLogItemsCollectionChanged();
        }

        public void OnFiltersChanged()
        {
            lock (_lastFilter)
            {
                this.ApplyFilterInternalNoCheck(_lastFilter);
            }
            this.NotifyLogItemsCollectionChanged();
        }

        public void ListenModuleFilterCheckedChangedEvent(ModuleFilterCheckboxValue moduleFilter)
        {
            moduleFilter._onCheckedStateChanged += this.OnFiltersChanged;
        }

        public void UnregisterFromModuleFilterCheckedChangedEvent(ModuleFilterCheckboxValue moduleFilter)
        {
            moduleFilter._onCheckedStateChanged -= this.OnFiltersChanged;
        }

        public void AddLogItem(LogItem item, ref List<string> newModuleAddedThisFrame)
        {
            _logItems.Add(item);

            if (!_moduleList.Any(moduleName => moduleName == item._moduleName))
            {
                _moduleList.Add(item._moduleName);
                newModuleAddedThisFrame.Add(item._moduleName);
            }
        }

        public void ClearLogs()
        {
            // Async way
            _shouldClearLogs = true;
        }

        private void DoClearLogsIfNeeded()
        {
            if (_isRunning && _shouldClearLogs)
            {
                _shouldClearLogs = false;

                if (_logItems.Count > 0)
                {
                    _logItems.Clear();

                    this.NotifyLogItemsCollectionChanged();
                }
            }
        }

        #endregion
    }
}
