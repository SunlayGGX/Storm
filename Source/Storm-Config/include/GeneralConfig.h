#pragma once


namespace Storm
{
	class MacroConfig;

	class GeneralConfig
	{
	public:
		GeneralConfig();

	public:
		bool read(const std::string &generalConfigFilePathStr);

		void applyMacros(const Storm::MacroConfig &macroConf);

	public:
		// Logger
		std::string _logFolderPath;
		std::string _logFileName;
		Storm::LogLevel _logLevel;
		bool _overrideLogs;
		int _shouldLogFPSWatching;
		int _removeLogsOlderThanDays;
	};
}
