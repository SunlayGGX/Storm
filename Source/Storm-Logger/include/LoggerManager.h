#pragma once

#include "Singleton.h"
#include "ILoggerManager.h"


namespace Storm
{
	class LogItem;
	enum class LogLevel;

	class LoggerManager final :
		private Storm::Singleton<LoggerManager>,
		public Storm::ILoggerManager
	{
		STORM_DECLARE_SINGLETON(LoggerManager);

	private:
		using LogArray = std::vector<Storm::LogItem>;

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		void log(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg) final override;
		Storm::LogLevel getLogLevel() const final override;

	private:
		void writeLogs(LogArray &logArray, const unsigned int currentPID) const;

	public:
		void logToTempFile(const std::string &fileName, const std::string &msg) const final override;

	private:
		bool _isRunning;
		std::thread _loggerThread;
		mutable std::mutex _loggerMutex;
		std::condition_variable _loggerCV;

		unsigned int _currentPID;

		Storm::LogLevel _level;

		LogArray _buffer;

		std::string _xmlLogFilePath;
		std::filesystem::path _tmpLogFolderPath;
	};
}
