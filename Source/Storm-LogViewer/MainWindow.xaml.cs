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

        public bool ShowEssentialCheckboxValue
        {
            get => ConfigManager.Instance.ShowEssentialOnly;
            set => ConfigManager.Instance.ShowEssentialOnly = value;
        }

        private List<GridViewColumn> _originalDisplayGridViewLayout = null;

        public MainWindow()
        {
            InitializeComponent();

            LogReaderManager loggerReaderMgr = LogReaderManager.Instance;

            LogDisplayArea.ItemsSource = loggerReaderMgr.DisplayedLogItems;

            FilterStrictEqualityCheckbox.DataContext = this;
            ShowEssentialOnlyCheckbox.DataContext = this;

            LogLevelsFilter.DataContext = this;
            LogLevelsFilter.ItemsSource = ConfigManager.Instance.LogLevelsFilter;

            ModuleLevelsFilter.DataContext = this;
            ModuleLevelsFilter.ItemsSource = ConfigManager.Instance.ModuleFilters;

            ConfigManager.Instance._onModuleFilterAdded += OnModuleFilterAdded;
            ConfigManager.Instance._onShowEssentialCheckboxChanged += UpdateListViewEssentiality;
            loggerReaderMgr._onDisplayedLogItemsCollectionChanged += OnDisplayedLogItemsCollectionChanged;
            loggerReaderMgr.NotifyLogItemsCollectionChanged();
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            ConfigManager.Instance._onModuleFilterAdded -= OnModuleFilterAdded;
            ConfigManager.Instance._onShowEssentialCheckboxChanged -= UpdateListViewEssentiality;
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

        void OnModuleFilterAdded(List<ModuleFilterCheckboxValue> newModuleCheckboxFilters)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                ModuleLevelsFilter.ItemsSource = newModuleCheckboxFilters;
                ICollectionView view = CollectionViewSource.GetDefaultView(ModuleLevelsFilter.ItemsSource);
                view.Refresh();
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

        public void UpdateListViewEssentiality(bool shouldShowEssential)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                GridView view = LogDisplayArea.View as GridView;
                if (view == null || view.Columns == null)
                {
                    return;
                }

                if (shouldShowEssential)
                {
                    if (_originalDisplayGridViewLayout == null)
                    {
                        _originalDisplayGridViewLayout = new List<GridViewColumn>(view.Columns);
                    }

                    List<GridViewColumn> toRemove = view.Columns.Where(column => 
                    {
                        if (column.Header as string == "Location")
                        {
                            return true;
                        }
                        else if (column.Header as string == "Thread")
                        {
                            return true;
                        }

                        return false;
                    }).ToList();

                    foreach (GridViewColumn toBeRemoved in toRemove)
                    {
                        view.Columns.Remove(toBeRemoved);
                    }
                }
                else if (_originalDisplayGridViewLayout != null)
                {
                    view.Columns.Clear();
                    foreach (GridViewColumn origColumns in _originalDisplayGridViewLayout)
                    {
                        view.Columns.Add(origColumns);
                    }
                }
            }));
        }
    }
}
