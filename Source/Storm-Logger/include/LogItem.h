#pragma once


namespace Storm
{
	enum class LogLevel;

	class LogItem
	{
	public:
		LogItem(const std::string_view &moduleName, const Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg);

	public:
		void prepare(bool xmlToo);

	public:
		const std::string& rawMessage() const;
		const std::string& toXml() const;

	public:
		static const std::string_view parseLogLevel(const Storm::LogLevel logLevel);

	public:
		const std::string_view _moduleName;
		const Storm::LogLevel _level;
		const std::string_view _function;
		const int _line;
		const std::chrono::system_clock::time_point _timestamp;
		const std::thread::id _threadId;
		const std::string _msg;

		std::string _finalMsg;
		std::string _finalXml;
	};
}
