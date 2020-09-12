using Storm_LogViewer.Source.Log;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace Storm_LogViewer
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public void StartUp(object sender, StartupEventArgs e)
        {
            Console.WriteLine("Starting Storm Log Reader Application");

            if (e.Args.Length == 0)
            {
                throw new System.Exception("We should have at least one argument which is the log file path!");
            }

            string logFilePath = e.Args.First(arg => arg.StartsWith("logfilepath", StringComparison.CurrentCultureIgnoreCase));
            logFilePath = logFilePath.Split('=')[1];

            LogReaderManager.Instance.Init(logFilePath);
        }
    }
}
