using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using System.Diagnostics;
using Storm_ScriptSender.Source.Script;
using Storm_ScriptSender.Source.Network;

namespace Storm_ScriptSender
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window//, INotifyPropertyChanged
    {
        //public event PropertyChangedEventHandler PropertyChanged;

        private ObservableCollection<ScriptItem> _scriptItemsList = new ObservableCollection<ScriptItem>();
        public ObservableCollection<ScriptItem> ScriptItemsList
        {
            get => _scriptItemsList;
        }


        public MainWindow()
        {
            InitializeComponent();

            ScriptsList.ItemsSource = ScriptItemsList;
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            Console.WriteLine("Ending Storm Log Reader Application");

            // Shutdown the application.
            Application.Current.Shutdown();
            // OR You can Also go for below logic
            // Environment.Exit(0);
        }

        //private void ExecuteOnUIThread(Action action)
        //{
        //    Application.Current.Dispatcher.InvokeAsync(action).Wait();
        //}

        private void ReorderScripts(int iter = 0)
        {
            int itemCount = ScriptItemsList.Count;
            for (; iter < itemCount; ++iter)
            {
                ScriptItemsList[iter].Index = iter;
            }
        }

        private void AddScriptItem(ScriptItem item)
        {
            item.Index = ScriptItemsList.Count;
            ScriptItemsList.Add(item);
        }

        private void RemoveScriptItem(ScriptItem item)
        {
            ScriptItemsList.RemoveAt(item.Index);
            ReorderScripts(item.Index);
        }

        private ScriptItem GetFromButtonSource(RoutedEventArgs e)
        {
            Button clickedCurrent = e.Source as Button;

            Debug.Assert(clickedCurrent != null, "Clicked current should be a Button!");

            ScriptItem parentScriptItem = clickedCurrent.Tag as ScriptItem;
            Debug.Assert(parentScriptItem != null, "Clicked current should be tagged with the parent script item using Tag in the xaml!");

            return parentScriptItem;
        }

        private void RemoveButton_Click(object sender, RoutedEventArgs e)
        {
            ScriptItem currentScriptItem = this.GetFromButtonSource(e);
            this.RemoveScriptItem(currentScriptItem);
            e.Handled = true;
        }

        private void SendButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void NewButton_Click(object sender, RoutedEventArgs e)
        {
            this.AddScriptItem(new ScriptItem());
            e.Handled = true;
        }
    }
}
