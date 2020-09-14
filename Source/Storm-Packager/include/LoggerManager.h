#pragma once

#include "Singleton.h"
#include "ILoggerManager.h"


namespace StormPackager
{
	class LoggerManager :
		private Storm::Singleton<StormPackager::LoggerManager>,
		public Storm::ILoggerManager
	{
		STORM_DECLARE_SINGLETON(LoggerManager);

	public:
		void log(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg) final override;
		Storm::LogLevel getLogLevel() const final override;
		void logToTempFile(const std::string &fileName, const std::string &msg) const final override;
	};
}
