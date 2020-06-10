#pragma once


#include "LogLevel.h"


namespace Storm
{
	class LoggerObject
	{
	public:
		LoggerObject(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line);
		~LoggerObject();

	private:
		template<class ToWriteType>
		static auto addToStream(std::stringstream &stream, const ToWriteType &toWrite, int) -> decltype(stream << toWrite)
		{
			return stream << toWrite;
		}

		template<class ToWriteType>
		static auto addToStream(std::stringstream &stream, const ToWriteType &toWrite, void*) -> decltype(stream << Storm::toStdString(toWrite))
		{
			return stream << Storm::toStdString(toWrite);
		}

	public:
		template<class ToWriteType>
		LoggerObject& operator<<(const ToWriteType &toWrite)
		{
			if (_enabled)
			{
				addToStream(_stream, toWrite, 0);
			}

			return *this;
		}

	private:
		const std::string_view _module;
		const Storm::LogLevel _level;
		const std::string_view _function;
		const int _line;
		const bool _enabled;
		std::stringstream _stream;
	};
}

#define STORM_LOG_BASE_IMPL(Level) Storm::LoggerObject{ STORM_MODULE_NAME, Level, __FUNCTION__, __LINE__ }

#define LOG_DEBUG           STORM_LOG_BASE_IMPL(Storm::LogLevel::Debug)
#define LOG_DEBUG_ERROR     STORM_LOG_BASE_IMPL(Storm::LogLevel::DebugError)
#define LOG_COMMENT         STORM_LOG_BASE_IMPL(Storm::LogLevel::Comment)
#define LOG_WARNING         STORM_LOG_BASE_IMPL(Storm::LogLevel::Warning)
#define LOG_ERROR           STORM_LOG_BASE_IMPL(Storm::LogLevel::Error)
#define LOG_FATAL           STORM_LOG_BASE_IMPL(Storm::LogLevel::Fatal)
#define LOG_ALWAYS          STORM_LOG_BASE_IMPL(Storm::LogLevel::Always)
