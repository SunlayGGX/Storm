using Storm_CsHelper.Source.Xml;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Linq;

namespace Storm_CsHelper.Source.Config
{
    class MacroConfig
    {
        #region Classes
        public class Macro
        {
            public string _key;
            public string _value;
        }

		private class RawBuiltInMacroRequest : IEquatable<RawBuiltInMacroRequest>
		{
			public string _key;
			public string _value;

			public bool Equals(RawBuiltInMacroRequest other)
			{
				return _key == other._key;
			}
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


		private void AddPrebuiltMacroInternal(params RawBuiltInMacroRequest[] builtInMacros)
		{
			// If no duplicate
			if (builtInMacros.Distinct().Count() != builtInMacros.Count())
			{
				throw new System.Exception("Macros requests should all be unique!");
			}

			foreach(RawBuiltInMacroRequest macroRequest in builtInMacros)
			{
				_macros.Add(new Macro { _key = MacroConfig.Macroify(macroRequest._key), _value = macroRequest._value });
			}

			// If all builtin macros were handled
			var macroTagsCteType = typeof(Storm.MacroTags);
			var allFields = macroTagsCteType.GetFields();
			foreach (var field in allFields)
			{
				if(field.FieldType == typeof(string) && field.IsStatic && field.IsLiteral)
				{
					if(builtInMacros.FirstOrDefault(macro => macro._key == field.GetValue(null) as string) == null)
					{
						throw new System.Exception("Not all builtin macros were handled!");
					}
				}
			}
		}

		private static string GetTmpPath(string outputPath)
		{
			return Directory.Exists(outputPath) ? outputPath : Path.Combine(Path.GetTempPath(), "Storm");
		}

		private static string GetCurrentLogViewerPID()
		{
			return System.Diagnostics.Process.GetCurrentProcess().Id.ToString();
		}

		public void AddPrebuiltMacro()
        {
            string exePath = System.Reflection.Assembly.GetExecutingAssembly().Location;
            string exeFolderPath = Path.GetDirectoryName(exePath);
            DirectoryInfo rootPathDirInfo = new DirectoryInfo(exeFolderPath).Parent;
            while (rootPathDirInfo != null && rootPathDirInfo.Name != "Storm")
            {
                rootPathDirInfo = rootPathDirInfo.Parent;
            }

            string rootPath = rootPathDirInfo.FullName;
            string outputPath = Path.Combine(rootPath, "Intermediate");

			this.AddPrebuiltMacroInternal(
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormExe,			_value = exePath },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormFolderExe,		_value = exeFolderPath },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormRoot,			_value = rootPath },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormConfig,		_value = Path.Combine(rootPath, "Config") },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormResource,		_value = Path.Combine(rootPath, "Resource") },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormIntermediate,	_value = outputPath },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormRecord,		_value = Path.Combine(outputPath, "Record") },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormStates,		_value = Path.Combine(outputPath, "States") },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormScripts,		_value = Path.Combine(outputPath, "Scripts") },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormDebug,			_value = Path.Combine(outputPath, "Debug") },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormArchive,		_value = Path.Combine(outputPath, "Archive") },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_StormTmp,			_value = MacroConfig.GetTmpPath(outputPath) },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_DateTime,			_value = DateTime.Now.ToString() },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_Date,				_value = DateTime.Now.ToLongDateString() },
				new RawBuiltInMacroRequest() { _key = Storm.MacroTags.k_builtInMacroKey_PID,				_value = MacroConfig.GetCurrentLogViewerPID() }
			);

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
