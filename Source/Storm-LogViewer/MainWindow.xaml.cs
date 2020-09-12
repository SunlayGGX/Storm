using Storm_LogViewer.Source.General.Config;
using Storm_LogViewer.Source.Log;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Storm_LogViewer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public bool FilterStrictEqualityCheckboxValue
        {
            get => ConfigManager.Instance.FilterStrictEquality;
            set => ConfigManager.Instance.FilterStrictEquality = value;
        }

        public MainWindow()
        {
            InitializeComponent();

            LogDisplayArea.ItemsSource = LogReaderManager.Instance.DisplayedLogItems;
            FilterStrictEqualityCheckbox.DataContext = this;

            LogReaderManager.Instance._onDisplayedLogItemsCollectionChanged += OnDisplayedLogItemsCollectionChanged;
            LogReaderManager.Instance.NotifyLogItemsCollectionChanged();
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            LogReaderManager.Instance._onDisplayedLogItemsCollectionChanged -= OnDisplayedLogItemsCollectionChanged;
            LogReaderManager.Instance.Shutdown();

            Console.WriteLine("Ending Storm Log Reader Application");

            // Shutdown the application.
            Application.Current.Shutdown();
            // OR You can Also go for below logic
            // Environment.Exit(0);
        }

        void OnDisplayedLogItemsCollectionChanged(List<LogItem> displayedLogItems)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                lock (displayedLogItems)
                {
                    LogDisplayArea.ItemsSource = displayedLogItems;
                    ICollectionView view = CollectionViewSource.GetDefaultView(LogDisplayArea.ItemsSource);
                    view.Refresh();
                }
            }));
        }

        private void LogFilterField_TextChanged(object sender, TextChangedEventArgs e)
        {
            if(sender is TextBox logFilterField)
            {
                LogReaderManager.Instance.ApplyFilter(LogFilterField.Text);
            }

            e.Handled = true;
        }

        protected void NotifyPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }
    }
}
