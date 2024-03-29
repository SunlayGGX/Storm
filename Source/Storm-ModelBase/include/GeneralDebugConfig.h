#pragma once


namespace Storm
{
	enum class VectoredExceptionDisplayMode;
	struct SocketSetting;

	struct GeneralDebugConfig
	{
	public:
		GeneralDebugConfig();
		~GeneralDebugConfig();

	public:
		// Logs
		std::string _srcLogFolderPath; // The macroized
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

		// PhysX
		std::unique_ptr<Storm::SocketSetting> _physXPvdDebugSocketSettings;
		unsigned int _pvdConnectTimeoutMillisec;
		bool _pvdTransmitConstraints;
		bool _pvdTransmitContacts;
		bool _pvdTransmitSceneQueries;

		// Graphics
		bool _keepUnsupported;
	};
}
