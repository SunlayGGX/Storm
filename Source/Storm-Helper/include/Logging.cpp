#include "Logging.h"

#include "SingletonHolder.h"
#include "ILoggerManager.h"


namespace
{
	Storm::ILoggerManager& retrieveLoggerManager()
	{
		return Storm::SingletonHolder::instance().getSingleton<Storm::ILoggerManager>();
	}

	std::stringstream& retrieveThreadLocalStream()
	{
		static thread_local std::stringstream stream;
		return stream;
	}
}


Storm::LoggerObject::LoggerObject(const std::string_view &moduleName, const Storm::LogLevel level, const std::string_view &function, const int line) :
	_stream{ retrieveThreadLocalStream() },
	_module{ moduleName },
	_level{ level },
	_function{ function },
	_line{ line },
	_enabled{ level >= retrieveLoggerManager().getLogLevel() }
{

}

Storm::LoggerObject::~LoggerObject()
{
	if (_enabled)
	{
		retrieveLoggerManager().log(_module, _level, _function, _line, _stream.str());
		_stream.str(std::string{});
	}
}
