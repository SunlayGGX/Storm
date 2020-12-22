using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Xml.Linq;
using Storm_LogViewer.Source.Converters;
using Storm_LogViewer.Source.General.Filterer;
using Storm_LogViewer.Source.Helpers;

namespace Storm_LogViewer.Source.Log
{
    class LogFileHandler
    {
        #region Members

        public System.DateTime _logFileCreationTime = DateTime.MinValue;
        public System.DateTime _logFileWriteTime = DateTime.MinValue;
        public long _lastStreamPos = 0;
        public FileInfo _logFileInfo = null;

        private bool _hasChangedOnce = false;

        #endregion

        #region Methods


        public bool TryParseOnce(ref bool isRunning, List<string> newModuleAddedThisFrame, List<uint> newPIDsAddedThisFrame)
        {
            return this.ParseLogFile(ref isRunning, newModuleAddedThisFrame, newPIDsAddedThisFrame);
        }

        private bool ParseLogFile(ref bool isRunning, List<string> newModuleAddedThisFrame, List<uint> newPIDsAddedThisFrame)
        {
            LogReaderManager logReaderMgr = LogReaderManager.Instance;

            XDocument doc = null;

            if (!isRunning)
            {
                return true;
            }

            try
            {
                _logFileInfo.Refresh();

                if (!_logFileInfo.Exists)
                {
                    logReaderMgr.RemoveWatcher(this);
                    return true;
                }

                // Up to date, nothing to do.
                if (_logFileInfo.LastWriteTime == _logFileWriteTime)
                {
                    return true;
                }

                _hasChangedOnce = true;

                // If the Storm logger is set to override the file, then the last stream pos is invalid.
                DateTime logFileCreationTime = _logFileInfo.CreationTime;
                if (_logFileCreationTime != logFileCreationTime)
                {
                    logReaderMgr.AddLogItem(new LogItem { _level = LogLevelEnum.NewSession }, ref newModuleAddedThisFrame, ref newPIDsAddedThisFrame);
                    logReaderMgr.RemoveWatcher(this);
                }

                int initialFileSize = (int)_logFileInfo.Length;
                using (FileStream filestream = new FileStream(_logFileInfo.FullName, FileMode.Open, FileAccess.Read, FileShare.ReadWrite, initialFileSize, FileOptions.RandomAccess))
                {
                    filestream.Position = _lastStreamPos <= initialFileSize ? _lastStreamPos : 0;

                    long tmpPosition;

                    string content = "<tmp>\n";
                    using (StreamReader reader = new StreamReader(filestream))
                    {
                        _logFileInfo.Refresh();
                        _logFileWriteTime = _logFileInfo.LastWriteTime;

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
                return false;
            }

            XmlHelper.LoadAnyElementsXMLFrom(doc.Root, elem =>
            {
                LogItem item = new LogItem { _level = LogLevelEnum.NewSession };

                if (elem.Name == "separator")
                {
                    logReaderMgr.AddLogItem(item, ref newModuleAddedThisFrame, ref newPIDsAddedThisFrame);
                }
                else if (!string.IsNullOrEmpty(elem.Value))
                {
                    elem.LoadAttributeIfExist("logLevel", value => item._level = LogLevelToStringConverter.FromString(value))
                        .LoadAttributeIfExist("module", value => item._moduleName = value)
                        .LoadAttributeIfExist("timestamp", value => item._timestamp = value)
                        .LoadAttributeIfExist("codeLocation", value => item._codeLocation = value)
                        .LoadAttributeIfExist("thread", value => item._threadId = value)
                        .LoadAttributeIfExist("PID", value => item._pid = uint.Parse(value))
                        ;
                    if (
                        !string.IsNullOrEmpty(item._moduleName) &&
                        !string.IsNullOrEmpty(item._timestamp) &&
                        !string.IsNullOrEmpty(item._codeLocation) &&
                        !string.IsNullOrEmpty(item._threadId)
                        )
                    {
                        item._msg = elem.Value;
                        logReaderMgr.AddLogItem(item, ref newModuleAddedThisFrame, ref newPIDsAddedThisFrame);
                    }
                }
            });

            return true;
        }

        public void Reset()
        {
            if (_hasChangedOnce)
            {
                _logFileWriteTime = DateTime.MinValue;
                _lastStreamPos = 0;
            }
        }

        #endregion
    }
}
