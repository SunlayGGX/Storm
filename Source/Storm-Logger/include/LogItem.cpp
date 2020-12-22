#include "LogItem.h"

#include "LogLevel.h"
#include "LogHelper.h"

#include "ThrowException.h"

#include <boost\property_tree\detail\xml_parser_write.hpp>
#include <boost\property_tree\xml_parser.hpp>


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

void Storm::LogItem::prepare(bool xmlToo, unsigned int processID)
{
	const std::string_view levelStr = Storm::parseLogLevel(_level);
	const std::string timestampStr = timeStampToString(_timestamp);
	const std::string codeLocationStr = _function + " - " + std::to_string(_line);
	const std::string lineStr = std::to_string(_line);
	const std::string threadIdStr = Storm::toStdString<LogThreadIdParserPolicy>(_threadId);

	const std::size_t levelStrSize = levelStr.size();
	const std::size_t timestampStrSize = timestampStr.size();
	const std::size_t codeLocationStrSize = codeLocationStr.size();
	const std::size_t threadIdStrSize = threadIdStr.size();
	const std::size_t msgSize = _msg.size();
	const std::size_t moduleNameStrSize = _moduleName.size();

	if (_finalMsg.empty())
	{
		_finalMsg.reserve(msgSize + moduleNameStrSize + codeLocationStrSize + levelStrSize + timestampStrSize + threadIdStrSize + 18);

		_finalMsg += '[';
		_finalMsg += levelStr;
		_finalMsg += "][";
		_finalMsg += _moduleName;
		_finalMsg += "][";
		_finalMsg += timestampStr;
		_finalMsg += "][";
		_finalMsg += codeLocationStr;
		_finalMsg += "][";
		_finalMsg += threadIdStr;
		_finalMsg += "] : ";

		_finalMsg += _msg;

		_finalMsg += '\n';
	}

	if (xmlToo && _finalXml.empty())
	{
		std::stringstream str;
		boost::property_tree::ptree logXml;

		logXml.add<decltype(levelStr)>("<xmlattr>.logLevel", levelStr);
		logXml.add<decltype(_moduleName)>("<xmlattr>.module", _moduleName);
		logXml.add<decltype(timestampStr)>("<xmlattr>.timestamp", timestampStr);
		logXml.add<decltype(threadIdStr)>("<xmlattr>.thread", threadIdStr);
		logXml.add<decltype(codeLocationStr)>("<xmlattr>.codeLocation", codeLocationStr);
		logXml.add<decltype(processID)>("<xmlattr>.PID", processID);
		logXml.put_value(_msg);

		boost::property_tree::xml_parser::write_xml_element(str, "log", logXml, 0, boost::property_tree::xml_writer_make_settings<std::string>('\n', 1));

		_finalXml = str.str();
	}
}

const std::string& Storm::LogItem::rawMessage() const
{
	assert(!_finalMsg.empty() && "The LogItem should have been prepared before coming here!");
	return _finalMsg;
}

const std::string& Storm::LogItem::toXml() const
{
	assert(!_finalXml.empty() && "The LogItem should have been prepared before coming here!");
	return _finalXml;
}
