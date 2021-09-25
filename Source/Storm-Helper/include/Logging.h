#pragma once


#include "LogLevel.h"


namespace Storm
{
	// No virtual destructor because the object is intended to be derived privately!
	class BaseLoggerObject
	{
	protected:
		BaseLoggerObject();

	protected:
		// Customs for types that would be declared later
		template<class ToWriteType>
		static auto addToStream(std::stringstream &stream, const ToWriteType &toWrite, int)
			-> decltype(std::enable_if_t<std::is_same_v<ToWriteType, Storm::Vector3>, std::true_type>::value, stream << Storm::toStdString(toWrite))
		{
			return stream << Storm::toStdString(toWrite);
		}

		template<class ToWriteType>
		static auto addToStream(std::stringstream &stream, const ToWriteType &toWrite, void*) -> decltype(stream << toWrite)
		{
			return stream << toWrite;
		}

		template<class ToWriteType>
		static auto addToStream(std::stringstream &stream, const ToWriteType &toWrite, ...) -> decltype(stream << Storm::toStdString(toWrite))
		{
			return stream << Storm::toStdString(toWrite);
		}

	protected:
		void clearStream();

	protected:
		std::stringstream &_stream;
	};

	class LoggerObject : private Storm::BaseLoggerObject
	{
	public:
		LoggerObject(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line);
		~LoggerObject();

	public:
		template<class ToWriteType>
		LoggerObject& operator<<(const ToWriteType &toWrite)
		{
			if (_enabled)
			{
				Storm::BaseLoggerObject::addToStream(_stream, toWrite, 0);
			}

			return *this;
		}

	private:
		const bool _enabled;
		const std::string_view _module;
		const Storm::LogLevel _level;
		const std::string_view _function;
		const int _line;
	};

	class FileLoggerObject : private Storm::BaseLoggerObject
	{
	public:
		FileLoggerObject(std::string filename);
		~FileLoggerObject();

	public:
		template<class ToWriteType>
		FileLoggerObject& operator<<(const ToWriteType &toWrite)
		{
			Storm::BaseLoggerObject::addToStream(_stream, toWrite, 0);
			return *this;
		}

	private:
		const std::string _filename;
	};
}

#define STORM_LOG_BASE_IMPL(Level) Storm::LoggerObject{ STORM_MODULE_NAME, Level, __FUNCTION__, __LINE__ }

#define LOG_DEBUG           STORM_LOG_BASE_IMPL(Storm::LogLevel::Debug)
#define LOG_DEBUG_WARNING   STORM_LOG_BASE_IMPL(Storm::LogLevel::DebugWarning)
#define LOG_DEBUG_ERROR     STORM_LOG_BASE_IMPL(Storm::LogLevel::DebugError)
#define LOG_SCRIPT_LOGIC    STORM_LOG_BASE_IMPL(Storm::LogLevel::ScriptLogic)
#define LOG_COMMENT         STORM_LOG_BASE_IMPL(Storm::LogLevel::Comment)
#define LOG_WARNING         STORM_LOG_BASE_IMPL(Storm::LogLevel::Warning)
#define LOG_ERROR           STORM_LOG_BASE_IMPL(Storm::LogLevel::Error)
#define LOG_FATAL           STORM_LOG_BASE_IMPL(Storm::LogLevel::Fatal)
#define LOG_ALWAYS          STORM_LOG_BASE_IMPL(Storm::LogLevel::Always)

#define DEBUG_LOG_UNTO_FILE(filename) Storm::FileLoggerObject{ filename }
