#pragma once


namespace Storm
{
	enum class LogLevel;

	std::string_view parseLogLevel(Storm::LogLevel logLevel);
}
