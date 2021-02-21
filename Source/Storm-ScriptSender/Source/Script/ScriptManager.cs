using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Storm_ScriptSender.Source.General.Config;
using System.Xml.Linq;
using Storm_CsHelper.Source.Xml;
using System.Collections.ObjectModel;

namespace Storm_ScriptSender.Source.Script
{
    class ScriptManager
    {
        private const string k_scriptContentXmlTag = "script";
        private const string k_scriptTabXmlTag = "Tab";
        private const string k_scriptTabTitleXmlAttributeTag = "Title";
        private const string k_selectedTabXmlAttributeTag = "SelectedIndex";


        private static ScriptManager s_instance = null;
        public static ScriptManager Instance
        {
            get
            {
                if (s_instance == null)
                {
                    s_instance = new ScriptManager();
                }

                return s_instance;
            }
        }


        public void SaveScripts(List<UIScriptTabItem> tabScriptsList, int selectedTabIndex)
        {
            XDocument doc = new XDocument();

            using (var xmlWriter = doc.CreateWriter())
            {
                xmlWriter.WriteStartElement("Scripts");
                xmlWriter.WriteAttributeString(k_selectedTabXmlAttributeTag, selectedTabIndex.ToString());

                foreach (UIScriptTabItem tabScripts in tabScriptsList)
                {
                    // For now, one tab to gather them all. Would prevent retro compatibility issues afterward when I'll implement them.
                    xmlWriter.WriteStartElement(k_scriptTabXmlTag);
                    xmlWriter.WriteAttributeString(k_scriptTabTitleXmlAttributeTag, tabScripts.Title);

                    foreach (ScriptItem scriptItem in tabScripts.Items)
                    {
                        xmlWriter.WriteStartElement(k_scriptContentXmlTag);
                        xmlWriter.WriteValue(scriptItem.ScriptTextContent);
                        xmlWriter.WriteEndElement();
                    }
                    xmlWriter.WriteEndElement();
                }

                xmlWriter.WriteEndElement();
                xmlWriter.Flush();
            }

            doc.Save(ConfigManager.Instance.ScriptXmlCompleteFilePath);

            Console.WriteLine("Script xml saved at " + ConfigManager.Instance.ScriptXmlCompleteFilePath);
        }

        public List<UIScriptTabItem> LoadScripts(out int selectedTabIndex)
        {
            selectedTabIndex = 0;

            List<UIScriptTabItem> result = new List<UIScriptTabItem>();

            FileInfo savedFile = new FileInfo(ConfigManager.Instance.ScriptXmlCompleteFilePath);
            if (savedFile.Exists)
            {
                XDocument doc = null;

                try
                {
                    using (FileStream filestream = new FileStream(savedFile.FullName, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                    {
                        string content;

                        using (StreamReader reader = new StreamReader(filestream))
                        {
                            content = reader.ReadToEnd();
                        }

                        doc = XDocument.Parse(content);
                    }
                }
                catch (System.Exception ex)
                {
                    doc = null;
                    Console.WriteLine("Script parsing failed, reason was " + ex.Message);
                }

                if (doc == null)
                {
                    return result;
                }

                int selectedTabIndexInLambdaCsWorkaround = 0;
                doc.Root.LoadAttributeAndThrowIfNotExist(k_selectedTabXmlAttributeTag, selectedIndexStr => selectedTabIndexInLambdaCsWorkaround = int.Parse(selectedIndexStr));
                selectedTabIndex = selectedTabIndexInLambdaCsWorkaround;

                XmlHelper.LoadAnyElementsXMLFrom(doc.Root, elem =>
                {
                    if (elem.Name == k_scriptContentXmlTag)
                    {
                        ScriptItem newScriptItem = new ScriptItem();

                        newScriptItem.ScriptTextContent = elem.Value;

                        UIScriptTabItem tabToAddInto;

                        if (result.Count == 0)
                        {
                            tabToAddInto = new UIScriptTabItem();
                            tabToAddInto.Title = "Default";
                            tabToAddInto.Items = new ObservableCollection<ScriptItem>();

                            result.Add(tabToAddInto);

                        }
                        else
                        {
                            tabToAddInto = result.Last();
                        }

                        newScriptItem.Index = tabToAddInto.Items.Count;
                        newScriptItem.ParentTab = tabToAddInto;

                        tabToAddInto.Items.Add(newScriptItem);
                    }
                    else if (elem.Name == k_scriptTabXmlTag)
                    {
                        string title = null;
                        elem.LoadAttributeAndThrowIfNotExist(k_scriptTabTitleXmlAttributeTag, uiTabValue => title = uiTabValue);

                        UIScriptTabItem tab = result.FirstOrDefault(uiTab => uiTab.Title == title);
                        if (tab == null)
                        {
                            tab = new UIScriptTabItem();
                            tab.Title = title;
                            tab.Items = new ObservableCollection<ScriptItem>();

                            result.Add(tab);
                        }

                        // For now, tabs aren't implemented, so it is ok to ignore it and proceed as if all scripts are gathered altogether.
                        // TODO : Change it when tabs would be implemented.
                        XmlHelper.LoadAnyElementsXMLFrom(elem, scriptElem =>
                        {
                            ScriptItem newScriptItem = new ScriptItem();

                            newScriptItem.ScriptTextContent = scriptElem.Value;
                            newScriptItem.Index = tab.Items.Count;
                            newScriptItem.ParentTab = tab;

                            tab.Items.Add(newScriptItem);
                        });
                    }
                });

                Console.WriteLine("Script xml loaded from " + ConfigManager.Instance.ScriptXmlCompleteFilePath + ". We have loaded " + result.Count + " script tabs.");
            }

            return result;
        }
    }
}
