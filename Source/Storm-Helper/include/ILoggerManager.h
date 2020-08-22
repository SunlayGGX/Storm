#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class LoggerObject;
	enum class LogLevel;

	class ILoggerManager : public Storm::ISingletonHeldInterface<ILoggerManager>
	{
	public:
		virtual ~ILoggerManager() = default;

	public:
		virtual void log(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg) = 0;
		virtual Storm::LogLevel getLogLevel() const = 0;

		virtual void logToTempFile(const std::string &fileName, const std::string &msg) const = 0;
	};
}
