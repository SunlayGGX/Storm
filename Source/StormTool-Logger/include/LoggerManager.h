#pragma once

#include "Singleton.h"
#include "ILoggerManager.h"
#include "SingletonDefaultImplementation.h"


namespace StormTool
{
	enum class LogLevel;

	class LoggerManager final :
		private Storm::Singleton<LoggerManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::ILoggerManager
	{
		STORM_DECLARE_SINGLETON(LoggerManager);

	public:
		void log(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg) final override;
		Storm::LogLevel getLogLevel() const final override;
		void logToTempFile(const std::string &fileName, const std::string &msg) const final override;
	};
}
