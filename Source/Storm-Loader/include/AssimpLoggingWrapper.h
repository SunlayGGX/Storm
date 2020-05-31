#pragma once

#include <Assimp/LogStream.hpp>


namespace Storm
{
	// Handle Assimp::Logger::ErrorSeverity::Debugging
	class AssimpDebugLoggerStream : public Assimp::LogStream
	{
	public:
		void write(const char* message) final override;
	};

	// Handle Assimp::Logger::ErrorSeverity::Info
	class AssimpInfoLoggerStream : public Assimp::LogStream
	{
	public:
		void write(const char* message) final override;
	};

	// Handle Assimp::Logger::ErrorSeverity::Warn
	class AssimpWarningLoggerStream : public Assimp::LogStream
	{
	public:
		void write(const char* message) final override;
	};

	// Handle Assimp::Logger::ErrorSeverity::Err
	class AssimpErrorLoggerStream : public Assimp::LogStream
	{
	public:
		void write(const char* message) final override;
	};
}
