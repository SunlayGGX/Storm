using Storm_LogViewer.Source.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace Storm_LogViewer.Source.General.Config
{
    class MacroConfig
    {
        #region Classes
        public class Macro
        {
            public string _key;
            public string _value;
        }

        #endregion

        #region Members

        private List<Macro> _macros = new List<Macro>();
        public List<Macro> Macros
        {
            get => _macros;
        }

        #endregion

        #region Methods

        #region Constructor
        public MacroConfig()
        {
            this.AddPrebuiltMacro();

            const string k_macroConfigFileName = "Macro.xml";
            string defaultGeneralConfigDirectoryFolder = Path.Combine(this.GetMacroEndValue("StormConfig"), "Custom", "General");
            string macroConfigPath = Path.Combine(defaultGeneralConfigDirectoryFolder, k_macroConfigFileName);
            if (File.Exists(macroConfigPath))
            {
                this.ReadFromXml(macroConfigPath);
            }
            else
            {
                macroConfigPath = Path.Combine(defaultGeneralConfigDirectoryFolder, "Original", k_macroConfigFileName);
                if (File.Exists(macroConfigPath))
                {
                    this.ReadFromXml(macroConfigPath);
                }
            }
        }

        public MacroConfig(string xmlPath)
        {
            this.AddPrebuiltMacro();
            this.ReadFromXml(this.Resolve(xmlPath));
        }

        #endregion

        #region Statics
        static public string Macroify(string name)
        {
            return "$[" + name + "]";
        }

        #endregion

        public void ReadFromXml(string xmlPath)
        {
            const string k_mainXmlTagName = "macros";
            const string k_macroTagName = "macro";
            const string k_macroKeyAttributeTagName = "key";
            const string k_macroValueAttributeTagName = "value";

            XElement xml = XElement.Load(xmlPath);
            if (xml != null)
            {
                if (xml.Name == k_mainXmlTagName)
                {
                    XmlHelper.LoadElementsXMLFrom(xml, k_macroTagName, element =>
                    {
                        Macro macroToAdd = new Macro();

                        element
                            .LoadAttributeAndThrowIfNotExist(k_macroKeyAttributeTagName, val => macroToAdd._key = MacroConfig.Macroify(val))
                            .LoadAttributeAndThrowIfNotExist(k_macroValueAttributeTagName, val => macroToAdd._value = val)
                            ;

                        _macros.Add(macroToAdd);
                    });
                }
                else
                {
                    throw new Exception("Macro xml config from " + xmlPath + " is invalid!");
                }
            }
            else
            {
                throw new Exception("Macro xml from " + xmlPath + " is null (for whatever reason)!");
            }

            int macroCount = _macros.Count;
            int lastMacroIndex = macroCount - 1;
            for (int iter = 0; iter < lastMacroIndex; ++iter)
            {
                Macro currentMacro = _macros[iter];
                for (int jiter = 0; jiter < macroCount; ++jiter)
                {
                    Macro testMacro = _macros[jiter];
                    if (currentMacro._key == testMacro._key && currentMacro._value != testMacro._value)
                    {
                        throw new Exception(
                            "Non unique macro detected, but with different values!\n" +
                            "key=" + currentMacro._key + "\n" +
                            "macro1='" + currentMacro._value + "'\n" +
                            "macro2='" + testMacro._value + "'\n"
                            );
                    }
                }
            }
        }

        public void AddPrebuiltMacro()
        {
            string exePath = System.Reflection.Assembly.GetExecutingAssembly().Location;
            string exeFolderPath = Path.GetDirectoryName(exePath);
            string rootPath = new DirectoryInfo(exeFolderPath).Parent.FullName;
            string outputPath = Path.Combine(rootPath, "Intermediate");

            _macros.Add(new Macro { _key = MacroConfig.Macroify("StormExe"), _value = exePath });
            _macros.Add(new Macro { _key = MacroConfig.Macroify("StormFolderExe"), _value = exeFolderPath });
            _macros.Add(new Macro { _key = MacroConfig.Macroify("StormRoot"), _value = rootPath });
            _macros.Add(new Macro { _key = MacroConfig.Macroify("StormConfig"), _value = Path.Combine(rootPath, "Config") });
            _macros.Add(new Macro { _key = MacroConfig.Macroify("StormResource"), _value = Path.Combine(rootPath, "Resource") });
            _macros.Add(new Macro { _key = MacroConfig.Macroify("StormIntermediate"), _value = outputPath });
            _macros.Add(new Macro { _key = MacroConfig.Macroify("DateTime"), _value = DateTime.Now.ToString() });
            _macros.Add(new Macro { _key = MacroConfig.Macroify("Date"), _value = DateTime.Now.ToLongDateString() });

            if (Directory.Exists(outputPath))
            {
                _macros.Add(new Macro { _key = MacroConfig.Macroify("StormTmp"), _value = outputPath });
            }
            else
            {
                DirectoryInfo tmpPath = new DirectoryInfo(Path.Combine(Path.GetTempPath(), "Storm"));
                if (!tmpPath.Exists)
                {
                    tmpPath.Create();
                }

                _macros.Add(new Macro { _key = MacroConfig.Macroify("StormTmp"), _value = tmpPath.FullName });
            }
        }

        private static bool MayContainMacro(string val)
        {
            return val.Contains("$[");
        }

        public string Resolve(string value)
        {
            string result = value;
            ResolveInPlace(ref result);
            return result;
        }

        public void ResolveInPlace(ref string value)
        {
            while (MayContainMacro(value))
            {
                foreach (Macro macro in _macros)
                {
                    value = value.Replace(macro._key, macro._value);
                }
            }
        }

        public string GetMacroEndValue(string rawKey)
        {
            string macroKey = MacroConfig.Macroify(rawKey);
            string result = Resolve(macroKey);
            return result != macroKey ? result : null;
        }

        #endregion

    }
}
