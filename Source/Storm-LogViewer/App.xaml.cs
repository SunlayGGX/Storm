using Storm_LogViewer.Source.General.Config;
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

            ConfigManager.Create(e.Args);
            LogReaderManager.Instance.Init();
        }
    }
}
