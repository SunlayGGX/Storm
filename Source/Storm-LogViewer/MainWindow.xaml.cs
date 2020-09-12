using Storm_LogViewer.Source.Log;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
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
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            LogDisplayArea.ItemsSource = LogReaderManager.Instance.LogItems;
            LogReaderManager.Instance._onLogItemsCollectionChanged += OnLogItemsCollectionChanged;
            LogReaderManager.Instance.NotifyLogItemsCollectionChanged();
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            LogReaderManager.Instance._onLogItemsCollectionChanged -= OnLogItemsCollectionChanged;
            LogReaderManager.Instance.Shutdown();

            Console.WriteLine("Ending Storm Log Reader Application");

            // Shutdown the application.
            Application.Current.Shutdown();
            // OR You can Also go for below logic
            // Environment.Exit(0);
        }

        void OnLogItemsCollectionChanged()
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                ICollectionView view = CollectionViewSource.GetDefaultView(LogDisplayArea.ItemsSource);
                view.Refresh();
            }));
        }
    }
}
