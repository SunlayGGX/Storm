#include "LogItem.h"

#include "LogLevel.h"
#include "ThrowException.h"

namespace
{
	const std::string timeStampToString(std::chrono::system_clock::time_point timeStamp)
	{
		std::string result;
		result.reserve(28);

		const auto time = std::chrono::system_clock::to_time_t(timeStamp);
		std::tm buf;
		localtime_s(&buf, &time);

		result += std::to_string(buf.tm_year + 1900);
		result += '/';
		result += std::to_string(buf.tm_mon);
		result += '/';
		result += std::to_string(buf.tm_mday);

		result += '-';

		result += std::to_string(buf.tm_hour);
		result += ':';
		result += std::to_string(buf.tm_min);
		result += ':';
		result += std::to_string(buf.tm_sec);
		result += ':';
		result += std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(timeStamp - std::chrono::time_point_cast<std::chrono::seconds>(timeStamp)).count());

		return result;
	}

	class LogThreadIdParserPolicy
	{
	public:
		template<class SrcPolicy>
		static std::string parse(const std::thread::id thId)
		{
			std::stringstream thStream;
			thStream << 'T' << thId << " (0x" << std::hex << thId << ')';
			return std::move(thStream).str();
		}
	};
}

Storm::LogItem::LogItem(const std::string_view &moduleName, const Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg) :
	_moduleName{ moduleName },
	_level{ level },
	_function{ function },
	_line{ line },
	_timestamp{ std::chrono::system_clock::now() },
	_threadId{ std::this_thread::get_id() },
	_msg{ std::move(msg) }
{

}

const std::string_view Storm::LogItem::parseLogLevel(const Storm::LogLevel logLevel)
{
#define STORM_SWITCH_CASE_STRINGIFY(CaseStatement) case Storm::LogLevel::CaseStatement: return #CaseStatement;
	switch (logLevel)
	{
		STORM_SWITCH_CASE_STRINGIFY(Debug);
		STORM_SWITCH_CASE_STRINGIFY(DebugError);
		STORM_SWITCH_CASE_STRINGIFY(Comment);
		STORM_SWITCH_CASE_STRINGIFY(Warning);
		STORM_SWITCH_CASE_STRINGIFY(Error);
		STORM_SWITCH_CASE_STRINGIFY(Fatal);
		STORM_SWITCH_CASE_STRINGIFY(Always);
	}
#undef STORM_SWITCH_CASE_STRINGIFY

	Storm::throwException<std::exception>("Unknown Level!");
}

Storm::LogItem::operator std::string() const
{
	std::string result;

	const std::string_view levelStr = Storm::LogItem::parseLogLevel(_level);
	const std::string timestampStr = timeStampToString(_timestamp);
	const std::string lineStr = std::to_string(_line);
	const std::string threadIdStr = Storm::toStdString<LogThreadIdParserPolicy>(_threadId);

	result.reserve(_msg.size() + _moduleName.size() + _function.size() + levelStr.size() + timestampStr.size() + lineStr.size() + threadIdStr.size() + 16);

	result += '[';
	result += levelStr;
	result += ']';

	result += '[';
	result += _moduleName;
	result += ']';

	result += '[';
	result += timestampStr;
	result += ']';

	result += '[';
	result += _function;
	result += " - ";
	result += lineStr;
	result += ']';

	result += '[';
	result += threadIdStr;
	result += "] : ";

	result += _msg;

	result += '\n';

	return result;
}
