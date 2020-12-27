#pragma once


namespace Storm
{
	class MacroConfig;
	enum class VectoredExceptionDisplayMode;
	enum class PreferredBrowser;

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
		bool _shouldLogFPSWatching;
		bool _shouldLogGraphicDeviceMessage;
		bool _shouldLogPhysics;
		int _removeLogsOlderThanDays;

		// Debug
		Storm::VectoredExceptionDisplayMode _displayVectoredExceptions;

		// Graphics
		unsigned int _wantedApplicationWidth;
		unsigned int _wantedApplicationHeight;
		int _wantedApplicationXPos;
		int _wantedApplicationYPos;
		float _fontSize;
		bool _fixNearFarPlanesWhenTranslating;
		bool _selectedParticleShouldBeTopMost;
		bool _selectedParticleForceShouldBeTopMost;

		// Web
		bool _urlOpenIncognito;
		Storm::PreferredBrowser _preferredBrowser;

		// Profile
		bool _profileSimulationSpeed;

		// Simulation
		bool _allowNoFluid;
	};
}
