#include "LoggerManager.h"
#include "LogLevel.h"
#include "LogHelper.h"

#include "LeanWindowsInclude.h"

#include "StormMacro.h"

#include <iostream>


StormPackager::LoggerManager::LoggerManager() = default;
StormPackager::LoggerManager::~LoggerManager() = default;

void StormPackager::LoggerManager::log(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg)
{
	const std::string_view levelStr = Storm::parseLogLevel(level);

	std::string totalMsg;
	totalMsg.reserve(16 + function.size() + moduleName.size() + msg.size() + levelStr.size());
	
	totalMsg += "[";
	totalMsg += levelStr;
	totalMsg += "][";
	totalMsg += moduleName;
	totalMsg += "][";
	totalMsg += function;
	totalMsg += " (";
	totalMsg += std::to_string(line);
	totalMsg += ")]: ";
	totalMsg += msg;
	totalMsg += "\n";

	std::cout << totalMsg;

	if (::IsDebuggerPresent())
	{
		::OutputDebugStringA(static_cast<LPCSTR>(totalMsg.c_str()));
	}
}

Storm::LogLevel StormPackager::LoggerManager::getLogLevel() const
{
	// No log level, we will log everything.
	return Storm::LogLevel::Debug;
}

void StormPackager::LoggerManager::logToTempFile(const std::string &fileName, const std::string &msg) const
{
	STORM_NOT_IMPLEMENTED;
}
