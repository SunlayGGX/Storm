using Storm_LogViewer.Source.Converters;
using Storm_LogViewer.Source.General.Config;
using Storm_LogViewer.Source.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Xml.Linq;
using Storm_LogViewer.Source.General.Filterer;

namespace Storm_LogViewer.Source.Log
{
    class LogReaderManager
    {
        #region Members

        private System.DateTime _lastLogFileWriteTime = DateTime.MinValue;
        private System.DateTime _lastLogFileCreationTime = DateTime.MinValue;
        private long _lastStreamPos = 0;

        private Thread _parserWatcherThread = null;

        private List<string> _moduleList = new List<string>(12);

        bool _shouldClearLogs = false;

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

        #endregion

        #region Events

        public delegate void OnDisplayedLogItemsCollectionChanged(List<LogItem> displayedLogItems, int maxLogCount);
        public event OnDisplayedLogItemsCollectionChanged _onDisplayedLogItemsCollectionChanged;

        #endregion


        #region Singleton

        private static LogReaderManager s_instance = null;
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
            ConfigManager configMgr = ConfigManager.Instance;
            configMgr._onFilterCheckboxChanged += this.OnFiltersChanged;

            FiltererManager filterMgr = FiltererManager.Instance;
            foreach (LogLevelFilterCheckboxValue checkboxValue in filterMgr.LogLevelsFilter)
            {
                checkboxValue._onCheckedStateChanged += this.OnFiltersChanged;
            }

            if (configMgr.NoInitialRead)
            {
                string logFileToRead = this.GetLogFileToRead();
                FileInfo logFileToReadInfo = new FileInfo(logFileToRead);

                _lastLogFileWriteTime = logFileToReadInfo.LastWriteTime;
                _lastLogFileCreationTime = logFileToReadInfo.CreationTime;
                _lastStreamPos = logFileToReadInfo.Length;
            }
            else
            {
                this.TryRunParsingOnce();
            }

            _parserWatcherThread = new Thread(() => this.Run());
            _parserWatcherThread.Start();
        }

        public void Shutdown()
        {
            _isRunning = false;
            _parserWatcherThread.Join();

            ConfigManager configMgr = ConfigManager.Instance;
            configMgr._onFilterCheckboxChanged -= this.OnFiltersChanged;

            FiltererManager filterMgr = FiltererManager.Instance;
            foreach (LogLevelFilterCheckboxValue checkboxValue in filterMgr.LogLevelsFilter)
            {
                checkboxValue._onCheckedStateChanged -= this.OnFiltersChanged;
            }
            foreach (ModuleFilterCheckboxValue checkboxValue in filterMgr.ModuleFilters)
            {
                checkboxValue._onCheckedStateChanged -= this.OnFiltersChanged;
            }
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

        private string GetLogFileToRead()
        {
            ConfigManager configMgr = ConfigManager.Instance;
            if (configMgr.HasLogFilePath)
            {
                return configMgr.LogFilePath;
            }
            else
            {
                return RetrieveLastLogFile();
            }
        }

        private void TryRunParsingOnce()
        {
            string logFilePath = this.GetLogFileToRead();

            if (_isRunning && logFilePath != null)
            {
                FileInfo fileInfo = new FileInfo(logFilePath);
                if (fileInfo.LastWriteTime > _lastLogFileWriteTime)
                {
                    this.ParseLogFile(fileInfo);
                }
            }
        }

        private void ParseLogFile(FileInfo logFileInfo)
        {
            XDocument doc = null;

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

                        if (_logItems.Count > 0)
                        {
                            _logItems.Add(new LogItem { _level = LogLevelEnum.NewSession });
                        }
                    }

                    int initialFileSize = (int)logFileInfo.Length;
                    using (FileStream filestream = new FileStream(logFileInfo.FullName, FileMode.Open, FileAccess.Read, FileShare.ReadWrite, initialFileSize, FileOptions.RandomAccess))
                    {
                        filestream.Position = _lastStreamPos;

                        long tmpPosition;

                        string content = "<tmp>\n";
                        using (StreamReader reader = new StreamReader(filestream))
                        {
                            logFileInfo.Refresh();
                            _lastLogFileWriteTime = logFileInfo.LastWriteTime;

                            content += reader.ReadToEnd();

                            tmpPosition = filestream.Position;
                        }

                        doc = XDocument.Parse(content + "\n</tmp>");
                        _lastStreamPos = tmpPosition;
                    }
                }
                catch (System.Exception ex)
                {
                    doc = null;
                    Console.WriteLine("Log parsing failed, reason was " + ex.Message);
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

            XmlHelper.LoadAnyElementsXMLFrom(doc.Root, elem => 
            {
                LogItem item = new LogItem { _level = LogLevelEnum.NewSession };

                if (elem.Name == "separator")
                {
                    this.AddLogItem(item, ref newModuleAddedThisFrame);
                }
                else if(!string.IsNullOrEmpty(elem.Value))
                {
                    elem.LoadAttributeIfExist("logLevel", value => item._level = LogLevelToStringConverter.FromString(value))
                        .LoadAttributeIfExist("module", value => item._moduleName = value)
                        .LoadAttributeIfExist("timestamp", value => item._timestamp = value)
                        .LoadAttributeIfExist("codeLocation", value => item._codeLocation = value)
                        .LoadAttributeIfExist("thread", value => item._threadId = value)
                        ;
                    if (
                        !string.IsNullOrEmpty(item._moduleName) &&
                        !string.IsNullOrEmpty(item._timestamp) &&
                        !string.IsNullOrEmpty(item._codeLocation) &&
                        !string.IsNullOrEmpty(item._threadId)
                        )
                    {
                        item._msg = elem.Value;
                        this.AddLogItem(item, ref newModuleAddedThisFrame);
                    }
                }
            });

            FiltererManager.Instance.AddNewModuleFilters(newModuleAddedThisFrame);

            if (preCollectionCount != _logItems.Count && _isRunning)
            {
                this.ApplyFilter();
            }
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

        private void GetLastLogFileFromFolder(string logFileFolder, ref FileInfo lastFile, ref DateTime lastDateTime)
        {
            const int k_watchdogLastCount = 10;

            DirectoryInfo logFolderInfo = new DirectoryInfo(logFileFolder);
            if (logFolderInfo.Exists)
            {
                foreach (FileInfo file in logFolderInfo.EnumerateFiles())
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
                                    return;
                                }

                                file.Refresh();
                            }

                        } while (++watchdog < k_watchdogLastCount);
                    }
                }
            }
        }

        public string RetrieveLastLogFile()
        {
            FileInfo lastFile = null;
            DateTime lastDateTime = DateTime.MinValue;

            this.GetLastLogFileFromFolder(Path.Combine(ConfigManager.Instance.MacrosConfig.GetMacroEndValue("StormIntermediate"), "Logs"), ref lastFile, ref lastDateTime);

            if (_isRunning && lastFile != null)
            {
                lastFile.Refresh();
                return lastFile.Exists ? lastFile.FullName : null;
            }
            else
            {
                return null;
            }
        }

        public void ApplyFilter(string newFilter)
        {
            _displayedLogItems = FiltererManager.Instance.ApplyFilters(_logItems, newFilter);
            this.NotifyLogItemsCollectionChanged();
        }

        public void ApplyFilter()
        {
            _displayedLogItems = FiltererManager.Instance.ApplyFilters(_logItems);
            this.NotifyLogItemsCollectionChanged();
        }

        public void OnFiltersChanged()
        {
            this.ApplyFilter();
        }

        public void ListenModuleFilterCheckedChangedEvent(ModuleFilterCheckboxValue moduleFilter)
        {
            moduleFilter._onCheckedStateChanged += this.OnFiltersChanged;
        }

        public void UnregisterFromModuleFilterCheckedChangedEvent(ModuleFilterCheckboxValue moduleFilter)
        {
            moduleFilter._onCheckedStateChanged -= this.OnFiltersChanged;
        }

        public void NotifyLogItemsCollectionChanged()
        {
            _onDisplayedLogItemsCollectionChanged?.Invoke(_displayedLogItems, _logItems.Count);
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
                    if (!object.ReferenceEquals(_logItems, _displayedLogItems))
                    {
                        _displayedLogItems.Clear();
                    }
                    this.NotifyLogItemsCollectionChanged();
                }
            }
        }

        #endregion
    }
}
