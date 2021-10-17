#include "SafetyHelpers.h"

#include "ExitCode.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"


namespace
{
	template<class TerminationHandler>
	__forceinline void sendTermination(const std::string_view msg, const TerminationHandler &callback)
	{
		LOG_FATAL << msg;
		// Leave time for the logger to process the last msg. After all, we waited x seconds (at least more than 30) if we come here, then 1 seconds more or less is nothing much.
		std::this_thread::sleep_for(std::chrono::seconds{ 1 });
		callback();
	}
}

void Storm::SafetyHelpers::sendCleanCloseTermination(const std::string_view msg)
{
	sendTermination(msg, []() { Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>().quit(); });
}

void Storm::SafetyHelpers::sendCleanExitTermination(const std::string_view msg)
{
	sendTermination(msg, []() { std::exit(static_cast<int>(Storm::ExitCode::k_safetyTermination)); });
}

void Storm::SafetyHelpers::sendHardKillTermination(const std::string_view msg)
{
	sendTermination(msg,std::terminate);
}
