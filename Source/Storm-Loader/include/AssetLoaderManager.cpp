#include "AssetLoaderManager.h"

#include "AssimpLoggingWrapper.h"

#include <Assimp\DefaultLogger.hpp>


namespace
{
	void initializeAssimpLogger()
	{
		Assimp::Logger* defaultLogger = Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
		if (!defaultLogger->attachStream(new Storm::AssimpDebugLoggerStream, Assimp::Logger::ErrorSeverity::Debugging))
		{
			LOG_WARNING << "Assimp Logger Debug stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpInfoLoggerStream, Assimp::Logger::ErrorSeverity::Info))
		{
			LOG_WARNING << "Assimp Logger Info stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpWarningLoggerStream, Assimp::Logger::ErrorSeverity::Warn))
		{
			LOG_WARNING << "Assimp Logger Warning stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpErrorLoggerStream, Assimp::Logger::ErrorSeverity::Err))
		{
			LOG_WARNING << "Assimp Logger Error stream failed to attach";
		}
	}
}



Storm::AssetLoaderManager::AssetLoaderManager() = default;
Storm::AssetLoaderManager::~AssetLoaderManager() = default;

void Storm::AssetLoaderManager::initialize_Implementation()
{
	initializeAssimpLogger();
}

void Storm::AssetLoaderManager::cleanUp_Implementation()
{
	Assimp::DefaultLogger::kill();
}
