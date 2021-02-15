using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Storm_ScriptSender.Source.General.Config;
using System.Xml.Linq;
using Storm_CsHelper.Source.Xml;

namespace Storm_ScriptSender.Source.Script
{
    class ScriptManager
    {
        private const string k_scriptContentXmlTag = "script";
        private const string k_scriptTabXmlTag = "Tab";


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


        public void SaveScripts(List<ScriptItem> scriptsList)
        {
            XDocument doc = new XDocument();

            using (var xmlWriter = doc.CreateWriter())
            {
                xmlWriter.WriteStartElement("Scripts");

                // For now, one tab to gather them all. Would prevent retro compatibility issues afterward when I'll implement them.
                xmlWriter.WriteStartElement(k_scriptTabXmlTag);
                foreach (ScriptItem scriptItem in scriptsList)
                {
                    xmlWriter.WriteStartElement(k_scriptContentXmlTag);
                    xmlWriter.WriteValue(scriptItem.ScriptTextContent);
                    xmlWriter.WriteEndElement();
                }
                xmlWriter.WriteEndElement();

                xmlWriter.WriteEndElement();
                xmlWriter.Flush();
            }

            doc.Save(ConfigManager.Instance.ScriptXmlCompleteFilePath);
        }

        public List<ScriptItem> LoadScripts()
        {
            List<ScriptItem> result = new List<ScriptItem>();

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

                XmlHelper.LoadAnyElementsXMLFrom(doc.Root, elem =>
                {
                    if (elem.Name == k_scriptContentXmlTag)
                    {
                        ScriptItem newScriptItem = new ScriptItem();

                        newScriptItem.ScriptTextContent = elem.Value;
                        newScriptItem.Index = result.Count;

                        result.Add(newScriptItem);
                    }
                    else if (elem.Name == k_scriptTabXmlTag)
                    {
                        // For now, tabs aren't implemented, so it is ok to ignore it and proceed as if all scripts are gathered altogether.
                        // TODO : Change it when tabs would be implemented.
                        XmlHelper.LoadAnyElementsXMLFrom(elem, scriptElem =>
                        {
                            ScriptItem newScriptItem = new ScriptItem();

                            newScriptItem.ScriptTextContent = scriptElem.Value;
                            newScriptItem.Index = result.Count;

                            result.Add(newScriptItem);
                        });
                    }
                });
            }

            return result;
        }
    }
}
