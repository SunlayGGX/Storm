using Storm_LogViewer.Source.General.Config;
using Storm_LogViewer.Source.General.Filterer;
using Storm_LogViewer.Source.Log;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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

        public bool AutoScrollCheckboxValue
        {
            get => ConfigManager.Instance.AutoScrollEnabled;
            set => ConfigManager.Instance.AutoScrollEnabled = value;
        }

        private List<GridViewColumn> _originalDisplayGridViewLayout = null;

        private string _logCountInfoStr;
        public string LogCountInfoStr
        {
            get => _logCountInfoStr;
            set
            {
                if (_logCountInfoStr != value)
                {
                    _logCountInfoStr = value;
                    NotifyPropertyChanged();
                }
            }
        }

        private ObservableCollection<LogItem> _displayedItemsSource = new ObservableCollection<LogItem>();

        private object _mutex = new object();

        public MainWindow()
        {
            InitializeComponent();

            ConfigManager configMgr = ConfigManager.Instance;
            FiltererManager filterMgr = FiltererManager.Instance;
            LogReaderManager loggerReaderMgr = LogReaderManager.Instance;

            LogDisplayArea.ItemsSource = _displayedItemsSource;

            BindingOperations.EnableCollectionSynchronization(LogDisplayArea.Items, _mutex);

            FilterStrictEqualityCheckbox.DataContext = this;
            ShowEssentialOnlyCheckbox.DataContext = this;
            AutoScrollCheckbox.DataContext = this;
            LogCountDisplayInfo.DataContext = this;

            LogLevelsFilter.DataContext = this;
            LogLevelsFilter.ItemsSource = filterMgr.LogLevelsFilter;

            ModuleLevelsFilter.DataContext = this;
            ModuleLevelsFilter.ItemsSource = filterMgr.ModuleFilters;

            filterMgr._onModuleFilterAdded += OnModuleFilterAdded;
            configMgr._onShowEssentialCheckboxChanged += UpdateListViewEssentiality;
            configMgr._onAutoScrollCheckboxChanged += AutoScrollUpdated;
            loggerReaderMgr._onDisplayedLogItemsCollectionChanged += OnDisplayedLogItemsCollectionChanged;
            loggerReaderMgr.NotifyLogItemsCollectionChanged();
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            ConfigManager configMgr = ConfigManager.Instance;
            FiltererManager filterMgr = FiltererManager.Instance;
            LogReaderManager loggerReaderMgr = LogReaderManager.Instance;

            filterMgr._onModuleFilterAdded -= OnModuleFilterAdded;
            configMgr._onShowEssentialCheckboxChanged -= UpdateListViewEssentiality;
            configMgr._onAutoScrollCheckboxChanged -= AutoScrollUpdated;
            loggerReaderMgr._onDisplayedLogItemsCollectionChanged -= OnDisplayedLogItemsCollectionChanged;
            loggerReaderMgr.Shutdown();

            Console.WriteLine("Ending Storm Log Reader Application");

            // Shutdown the application.
            Application.Current.Shutdown();
            // OR You can Also go for below logic
            // Environment.Exit(0);
        }

        void OnDisplayedLogItemsCollectionChanged(List<LogItem> displayedLogItems, int maxLogCount)
        {
            int displayedLogCount = displayedLogItems.Count;
            string logInfo;
            if (maxLogCount != 0)
            {
                logInfo = displayedLogCount + "/" + maxLogCount + " logs displayed (" + ((float)displayedLogCount / (float)maxLogCount).ToString("P3") + ")";
            }
            else
            {
                logInfo = "0/0 logs displayed";
            }

            ExecuteOnUIThread(new Action(() =>
            {
                lock (_mutex)
                {
                    LogCountInfoStr = logInfo;

                    _displayedItemsSource.Clear();
                    foreach (LogItem item in displayedLogItems)
                    {
                        _displayedItemsSource.Add(item);
                    }
                    /*LogDisplayArea.ItemsSource = null;
                    LogDisplayArea.ItemsSource = displayedLogItems;*/

                    ScrollToEndIfAutoScroll_UIThread();
                }
            }));
        }

        void OnModuleFilterAdded(List<ModuleFilterCheckboxValue> newModuleCheckboxFilters)
        {
            ExecuteOnUIThread(new Action(() =>
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
            ExecuteOnUIThread(new Action(() =>
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

        private void ScrollToEndIfAutoScroll_UIThread()
        {
            if (ConfigManager.Instance.AutoScrollEnabled)
            {
                int itemCount = LogDisplayArea.Items.Count;
                if (itemCount > 0)
                {
                    try
                    {
                        LogDisplayArea.ScrollIntoView(LogDisplayArea.Items[itemCount - 1]);
                    }
                    catch (System.Exception ex)
                    {
                        Console.WriteLine("Exception happened while scrolling to the end : " + ex.Message + ".\nStack Trace : " + ex.StackTrace);
                    }
                }
            }
        }

        public void AutoScrollUpdated()
        {
            ExecuteOnUIThread(new Action(() =>
            {
                lock (_mutex)
                {
                    ScrollToEndIfAutoScroll_UIThread();
                }
            }));
        }

        private void ClearButton_Click(object sender, RoutedEventArgs e)
        {
            LogReaderManager.Instance.ClearLogs();

            e.Handled = true;
        }

        private void ExecuteOnUIThread(Action action)
        {
            Application.Current.Dispatcher.InvokeAsync(action).Wait();
        }
    }
}
