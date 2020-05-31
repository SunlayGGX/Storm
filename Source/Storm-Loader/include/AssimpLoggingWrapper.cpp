#include "AssimpLoggingWrapper.h"


void Storm::AssimpDebugLoggerStream::write(const char* message)
{
	LOG_DEBUG << "Assimp message : " << message;
}

void Storm::AssimpInfoLoggerStream::write(const char* message)
{
	LOG_COMMENT << "Assimp message : " << message;
}

void Storm::AssimpWarningLoggerStream::write(const char* message)
{
	LOG_WARNING << "Assimp message : " << message;
}

void Storm::AssimpErrorLoggerStream::write(const char* message)
{
	LOG_ERROR << "Assimp message : " << message;
}
