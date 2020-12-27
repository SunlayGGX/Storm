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

        private System.DateTime _lastLogFileTime = DateTime.MinValue;
        private List<LogFileHandler> _watchedLogFiles = new List<LogFileHandler>(4);
        private List<LogFileHandler> _watcherToRemove = new List<LogFileHandler>(4);

        private Thread _parserWatcherThread = null;

        private List<string> _moduleList = new List<string>(12);
        private List<uint> _pidsList = new List<uint>(4);

        bool _shouldClearLogs = false;
        bool _shouldReReadLogs = false;

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

            FileInfo logFileToReadInfo = null;
            if (configMgr.ReadLast || configMgr.HasLogFilePath)
            {
                string logFileToRead = this.GetLogFileToRead();
                logFileToReadInfo = new FileInfo(logFileToRead);

                _watchedLogFiles.Add(new LogFileHandler()
                {
                    _logFileWriteTime = DateTime.MinValue,
                    _logFileCreationTime = logFileToReadInfo.CreationTime,
                    _lastStreamPos = 0,
                    _logFileInfo = logFileToReadInfo
                });

                _lastLogFileTime = logFileToReadInfo.LastWriteTime;

                this.GetLastLogFileFromFolder(LogReaderManager.GetIntermediateLogFolderPath(), ref logFileToReadInfo, ref _lastLogFileTime);
                this.GetLastLogFileFromFolder(LogReaderManager.GetTempLogFolderPath(), ref logFileToReadInfo, ref _lastLogFileTime);
            }
            else
            {
                this.FillNewWatchedFiles(true);
            }

            _parserWatcherThread = new Thread(() => this.Run());
            _parserWatcherThread.Start();

            Console.WriteLine("Log parsing thread started!");
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
                Thread.Sleep(1000);

                if (_isRunning)
                {
                    DoClearLogsIfNeeded();
                    DoResetIfNeeded();

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
            if (_isRunning)
            {
                List<string> newModuleAddedThisFrame = new List<string>(12);
                List<uint> newPIDsAddedThisFrame = new List<uint>(2);

                this.FillNewWatchedFiles();

                List<LogFileHandler> toExecute = new List<LogFileHandler>(_watchedLogFiles.Count);
                toExecute.AddRange(_watchedLogFiles);

                int preCollectionCount = _logItems.Count;

                int watchdog = 10;

                while (toExecute.Count > 0)
                {
                    if (!_isRunning)
                    {
                        return;
                    }

                    List<LogFileHandler> completed = new List<LogFileHandler>(toExecute.Count);

                    foreach (LogFileHandler watchedLogFile in toExecute)
                    {
                        if (_isRunning)
                        {
                            if (watchedLogFile.TryParseOnce(ref _isRunning, newModuleAddedThisFrame, newPIDsAddedThisFrame))
                            {
                                completed.Add(watchedLogFile);
                            }
                        }
                        else
                        {
                            return;
                        }
                    }

                    if (completed.Count > 0)
                    {
                        toExecute.RemoveAll(watched => completed.Contains(watched) || _watcherToRemove.Contains(watched));
                    }
                    else
                    {
                        Thread.Sleep(100);
                        if (--watchdog == 0)
                        {
                            break;
                        }
                    }
                }

                if (_isRunning)
                {
                    FiltererManager filterMgr = FiltererManager.Instance;
                    filterMgr.AddNewModuleFilters(newModuleAddedThisFrame);
                    filterMgr.AddNewPIDFilters(newPIDsAddedThisFrame);

                    if (preCollectionCount != _logItems.Count && _isRunning)
                    {
                        this.ApplyFilter();
                    }
                }
            }
        }

        private void FillNewWatchedFileFromFolder(string logFileFolder, ref DateTime maxLogFileTime, bool init)
        {
            const int k_watchdogLastCount = 10;

            ConfigManager configMgr = ConfigManager.Instance;

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
                                DateTime currentLastFileTime = file.LastWriteTime;
                                if (currentLastFileTime > _lastLogFileTime)
                                {
                                    if (!_watchedLogFiles.Any(watchedLogFile => watchedLogFile._logFileInfo.FullName == file.FullName))
                                    {
                                        // This is a completely new log file.
                                        if (file.CreationTime > _lastLogFileTime && !init)
                                        {
                                            _watchedLogFiles.Add(new LogFileHandler()
                                            {
                                                _logFileWriteTime = DateTime.MinValue,
                                                _logFileCreationTime = file.CreationTime,
                                                _lastStreamPos = 0,
                                                _logFileInfo = file
                                            });
                                        }
                                        else // The file existed before but wasn't watched. We just catch on.
                                        {
                                            if (configMgr.NoInitialRead)
                                            {
                                                _watchedLogFiles.Add(new LogFileHandler()
                                                {
                                                    _logFileWriteTime = file.LastWriteTime,
                                                    _logFileCreationTime = file.CreationTime,
                                                    _lastStreamPos = file.Length,
                                                    _logFileInfo = file
                                                });
                                            }
                                            else if (!init)
                                            {
                                                _watchedLogFiles.Add(new LogFileHandler()
                                                {
                                                    _logFileWriteTime = DateTime.MinValue,
                                                    _logFileCreationTime = file.CreationTime,
                                                    _lastStreamPos = 0,
                                                    _logFileInfo = file
                                                });
                                            }
                                        }
                                    }

                                    if (currentLastFileTime > maxLogFileTime)
                                    {
                                        maxLogFileTime = currentLastFileTime;
                                    }
                                }
                                else
                                {
                                    break;
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

        private void FillNewWatchedFiles(bool init = false)
        {
            DateTime maxLogFileTime = _lastLogFileTime;

            FillNewWatchedFileFromFolder(LogReaderManager.GetIntermediateLogFolderPath(), ref maxLogFileTime, init);
            FillNewWatchedFileFromFolder(LogReaderManager.GetTempLogFolderPath(), ref maxLogFileTime, init);

            _lastLogFileTime = maxLogFileTime;
        }

        public void AddLogItem(LogItem item, ref List<string> newModuleAddedThisFrame, ref List<uint> newPIDsAddedThisFrame)
        {
            _logItems.Add(item);

            if (!_moduleList.Any(moduleName => moduleName == item._moduleName))
            {
                _moduleList.Add(item._moduleName);
                newModuleAddedThisFrame.Add(item._moduleName);
            }

            if (!_pidsList.Any(pid => pid == item._pid))
            {
                _pidsList.Add(item._pid);
                newPIDsAddedThisFrame.Add(item._pid);
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

            this.GetLastLogFileFromFolder(LogReaderManager.GetIntermediateLogFolderPath(), ref lastFile, ref lastDateTime);
            this.GetLastLogFileFromFolder(LogReaderManager.GetTempLogFolderPath(), ref lastFile, ref lastDateTime);

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

        public void ListenPIDFilterCheckedChangedEvent(PIDFilterCheckboxValue pidFilter)
        {
            pidFilter._onCheckedStateChanged += this.OnFiltersChanged;
        }

        public void UnregisterFromPIDFilterCheckedChangedEvent(PIDFilterCheckboxValue pidFilter)
        {
            pidFilter._onCheckedStateChanged -= this.OnFiltersChanged;
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

        public void ReadLogsFromBeginning()
        {
            // Async way
            _shouldReReadLogs = true;
        }

        private void DoResetIfNeeded()
        {
            if (_isRunning && _shouldReReadLogs)
            {
                _shouldReReadLogs = false;

#if false
                _lastLogFileCreationTime = DateTime.MinValue;
                _lastLogFileWriteTime = DateTime.MinValue;
                _lastStreamPos = 0;
#else
                foreach (LogFileHandler fileWatcher in _watchedLogFiles)
                {
                    fileWatcher.Reset();
                }
#endif
            }
        }

        internal void RemoveWatcher(LogFileHandler logFileHandler)
        {
            int index = _watchedLogFiles.IndexOf(logFileHandler);
            if (index != -1)
            {
                _watcherToRemove.Add(logFileHandler);
                _watchedLogFiles.RemoveAt(index);
            }
        }

        private static string GetIntermediateLogFolderPath()
        {
            return Path.Combine(ConfigManager.Instance.MacrosConfig.GetMacroEndValue("StormIntermediate"), "Logs");
        }

        private static string GetTempLogFolderPath()
        {
            return Path.Combine(Path.GetTempPath(), "Storm", "Logs");
        }

        #endregion
    }
}
