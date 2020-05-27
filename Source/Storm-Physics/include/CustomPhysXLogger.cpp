#include "CustomPhysXLogger.h"

#include "ExitCode.h"


void Storm::CustomPhysXLogger::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
#define STORM_LOG_CASE(pxErrorCode, loggingMacro) case physx::PxErrorCode::pxErrorCode: loggingMacro << "PhysX log callback (" << #pxErrorCode " - " << file << " - " << line << ") : " << message; break

	switch (code)
	{
		STORM_LOG_CASE(eNO_ERROR, LOG_COMMENT);
		STORM_LOG_CASE(eDEBUG_INFO, LOG_DEBUG);
		STORM_LOG_CASE(eDEBUG_WARNING, LOG_WARNING);
		STORM_LOG_CASE(eINVALID_PARAMETER, LOG_ERROR);
		STORM_LOG_CASE(eINVALID_OPERATION, LOG_ERROR);
		STORM_LOG_CASE(eOUT_OF_MEMORY, LOG_FATAL);
		STORM_LOG_CASE(eINTERNAL_ERROR, LOG_ERROR);
		STORM_LOG_CASE(eABORT, LOG_FATAL);
		STORM_LOG_CASE(ePERF_WARNING, LOG_WARNING);
		STORM_LOG_CASE(eMASK_ALL, LOG_ERROR);

	default:
		break;
	}

	if (code == physx::PxErrorCode::eABORT)
	{
		LOG_FATAL << "Abort was called, we would exit the application!";

		__debugbreak();
		std::this_thread::sleep_for(std::chrono::seconds{ 1 });
		exit(static_cast<int>(Storm::ExitCode::k_termination));
	}

#undef STORM_LOG_CASE
}
