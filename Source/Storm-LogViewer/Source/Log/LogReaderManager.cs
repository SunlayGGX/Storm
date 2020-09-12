using Storm_LogViewer.Source.Converters;
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

        private string _logFilePath = null;
        public string LogFilePath
        {
            get => _logFilePath;
        }

        private System.DateTime _lastLogFileWriteTime;

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

        Thread _parserWatcherThread = null;

        long _lastStreamPos = 0;

        #endregion

        #region Events

        public delegate void OnLogItemsCollectionChanged();
        public event OnLogItemsCollectionChanged _onLogItemsCollectionChanged;

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

        public LogReaderManager()
        {
        }

        #endregion

        public void Init(string logFilePath)
        {
            _logFilePath = logFilePath;

            ParseLogFile(new FileInfo(logFilePath));

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

        private void ParseLogFile(FileInfo fileInfo)
        {
            XmlDocument doc = null;

            int initialFileSize = (int)fileInfo.Length;
            using (FileStream filestream = new FileStream(_logFilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite, initialFileSize, FileOptions.RandomAccess))
            {
                filestream.Position = _lastStreamPos;

                string content = "<tmp>\n";
                do
                {
                    using (StreamReader reader = new StreamReader(filestream))
                    {
                        fileInfo.Refresh();
                        _lastLogFileWriteTime = fileInfo.LastWriteTime;

                        content += reader.ReadToEnd();

                        _lastStreamPos = filestream.Position;
                    }

                    doc = new XmlDocument();

                    try
                    {
                        doc.LoadXml(content + "</tmp>");
                    }
                    catch (System.Exception)
                    {
                        doc = null;
                    }

                    if (!_isRunning)
                    {
                        return;
                    }

                } while (doc == null);
            }

            int preCollectionCount = _logItems.Count;

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
                            _logItems.Add(item);
                        }
                    }
                }
                else
                {
                    _logItems.Add(item);
                }
            }

            if (preCollectionCount != _logItems.Count && _isRunning)
            {
                this.NotifyLogItemsCollectionChanged();
            }
        }

        public void NotifyLogItemsCollectionChanged()
        {
            _onLogItemsCollectionChanged?.Invoke();
        }

        private void Run()
        {
            while (_isRunning)
            {
                Thread.Sleep(500);

                if (_isRunning)
                {
                    FileInfo fileInfo = new FileInfo(_logFilePath);
                    if (fileInfo.LastWriteTime != _lastLogFileWriteTime)
                    {
                        this.ParseLogFile(fileInfo);
                    }
                }
            }
        }

        public void Shutdown()
        {
            _isRunning = false;
            _parserWatcherThread.Join();
        }


        #endregion
    }
}
