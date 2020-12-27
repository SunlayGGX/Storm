#pragma once


namespace Storm
{
	enum class VectoredExceptionDisplayMode;

	struct GeneralDebugConfig
	{
	public:
		GeneralDebugConfig();

	public:
		// Logs
		std::string _logFolderPath;
		std::string _logFileName;
		Storm::LogLevel _logLevel;
		bool _overrideLogs;
		bool _shouldLogFPSWatching;
		bool _shouldLogGraphicDeviceMessage;
		bool _shouldLogPhysics;
		int _removeLogsOlderThanDays;

		// Exceptions
		Storm::VectoredExceptionDisplayMode _displayVectoredExceptions;

		// Profile
		bool _profileSimulationSpeed;
	};
}
