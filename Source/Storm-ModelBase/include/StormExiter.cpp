#include "StormExiter.h"

#include "ISimulatorManager.h"
#include "SingletonHolder.h"

#include "ExitCode.h"


void Storm::requestExitOtherThread()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::ISimulatorManager>().exitWithCode(Storm::ExitCode::k_otherThreadTermination);
}
