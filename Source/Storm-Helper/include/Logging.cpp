#include "Logging.h"

#include "SingletonHolder.h"
#include "ILoggerManager.h"


namespace
{
	Storm::ILoggerManager* retrieveLoggerManager()
	{
		return Storm::SingletonHolder::instance().getFacet<Storm::ILoggerManager>();
	}

	std::stringstream& retrieveThreadLocalStream()
	{
		static thread_local std::stringstream stream;
		return stream;
	}

	__forceinline bool isEnabled(const Storm::LogLevel level)
	{
		const auto*const optionalLoggerMgr = retrieveLoggerManager();
		return optionalLoggerMgr && optionalLoggerMgr->getLogLevel() <= level;
	}
}

Storm::BaseLoggerObject::BaseLoggerObject() :
	_stream{ retrieveThreadLocalStream() }
{

}

void Storm::BaseLoggerObject::clearStream()
{
	_stream.str(std::string{});
}

Storm::LoggerObject::LoggerObject(const std::string_view &moduleName, const Storm::LogLevel level, const std::string_view &function, const int line) :
	_module{ moduleName },
	_level{ level },
	_function{ function },
	_line{ line },
	_enabled{ isEnabled(level) }
{

}

Storm::LoggerObject::~LoggerObject()
{
	if (_enabled)
	{
		retrieveLoggerManager()->log(_module, _level, _function, _line, std::move(_stream).str());
	}
	Storm::BaseLoggerObject::clearStream();
}


Storm::FileLoggerObject::FileLoggerObject(std::string filename) :
	_filename{ std::move(filename) }
{

}

Storm::FileLoggerObject::~FileLoggerObject()
{
	const auto*const optionalLoggerMgr = retrieveLoggerManager();
	if (optionalLoggerMgr)
	{
		optionalLoggerMgr->logToTempFile(_filename, std::move(_stream).str());
	}
	Storm::BaseLoggerObject::clearStream();
}
