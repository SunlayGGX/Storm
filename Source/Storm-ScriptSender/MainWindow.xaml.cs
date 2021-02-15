using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using System.Diagnostics;
using Storm_ScriptSender.Source.Script;
using Storm_ScriptSender.Source.Network;
using System.Collections.Generic;

namespace Storm_ScriptSender
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window//, INotifyPropertyChanged
    {
        //public event PropertyChangedEventHandler PropertyChanged;

        private ObservableCollection<UIScriptTabItem> _tabsScriptItemsList;
        public ObservableCollection<UIScriptTabItem> TabsScriptItemsList
        {
            get => _tabsScriptItemsList;
        }


        public MainWindow()
        {
            InitializeComponent();

            int selectedTabIndex;
            List<UIScriptTabItem> tabScriptsItemList = ScriptManager.Instance.LoadScripts(out selectedTabIndex);
            _tabsScriptItemsList = new ObservableCollection<UIScriptTabItem>(tabScriptsItemList);
            
            if (TabsScriptItemsList.Count == 0)
            {
                TabsScriptItemsList.Add(new UIScriptTabItem()
                {
                    Title = "Default",
                    Items = new ObservableCollection<ScriptItem>()
                });
            }

            TabScriptsList.SelectedIndex = selectedTabIndex < TabsScriptItemsList.Count ? selectedTabIndex : 0;
            TabScriptsList.ItemsSource = TabsScriptItemsList;
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            Console.WriteLine("Ending Storm Log Reader Application");

            NetworkManager.Instance.Close();

            // Shutdown the application.
            Application.Current.Shutdown();
            // OR You can Also go for below logic
            // Environment.Exit(0);
        }

        //private void ExecuteOnUIThread(Action action)
        //{
        //    Application.Current.Dispatcher.InvokeAsync(action).Wait();
        //}

        private void ReorderScripts(int tab, int iter = 0)
        {
            UIScriptTabItem currentTab = TabsScriptItemsList[tab];
            int itemCount = currentTab.Items.Count;
            for (; iter < itemCount; ++iter)
            {
                currentTab.Items[iter].Index = iter;
            }
        }

        private void AddScriptItem(ScriptItem item)
        {
            UIScriptTabItem currentTab = TabsScriptItemsList[TabScriptsList.SelectedIndex];
            item.Index = currentTab.Items.Count;
            item.ParentTab = currentTab;

            currentTab.Items.Add(item);
        }

        private void AddNewTabItem()
        {
            string tabRootName = "Default";

            UIScriptTabItem added = new UIScriptTabItem();
            added.Title = tabRootName;
            added.Items = new ObservableCollection<ScriptItem>();
            int index = 0;
            do
            {
                bool found = false;
                foreach (var tabItem in TabsScriptItemsList)
                {
                    if (tabItem.Title == added.Title)
                    {
                        found = true;
                        break;
                    }
                }

                if (found)
                {
                    added.Title = tabRootName + "_" + index.ToString();
                    ++index;
                }
                else
                {
                    break;
                }
            } while (true);

            TabScriptsList.SelectedIndex = TabsScriptItemsList.Count;
            TabsScriptItemsList.Add(added);
        }

        private void RemoveScriptItem(ScriptItem item)
        {
            _tabsScriptItemsList[TabScriptsList.SelectedIndex].Items.RemoveAt(item.Index);
            ReorderScripts(TabScriptsList.SelectedIndex, item.Index);
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
            ScriptItem currentScriptItem = this.GetFromButtonSource(e);
            NetworkManager.Instance.SendScript(currentScriptItem);
            e.Handled = true;
        }

        private void NewScriptButton_Click(object sender, RoutedEventArgs e)
        {
            this.AddScriptItem(new ScriptItem());
            e.Handled = true;
        }

        private void NewTab_Click(object sender, RoutedEventArgs e)
        {
            this.AddNewTabItem();
            e.Handled = true;
        }

        private void RemoveTabButton_Click(object sender, RoutedEventArgs e)
        {
            if (_tabsScriptItemsList.Count == 1)
            {
                return;
            }

            Button clickedCurrent = e.Source as Button;
            Debug.Assert(clickedCurrent != null, "Clicked current should be a Button!");

            UIScriptTabItem tabToRemove = clickedCurrent.Tag as UIScriptTabItem;
            Debug.Assert(tabToRemove != null, "Clicked current should be tagged with the parent script tab using Tag in the xaml!");

            _tabsScriptItemsList.Remove(tabToRemove);
            e.Handled = true;
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            List<UIScriptTabItem> items = new List<UIScriptTabItem>(_tabsScriptItemsList);
            ScriptManager.Instance.SaveScripts(items, TabScriptsList.SelectedIndex);
            e.Handled = true;
        }
    }
}
