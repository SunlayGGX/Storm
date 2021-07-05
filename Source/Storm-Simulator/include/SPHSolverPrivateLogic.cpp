#include "SPHSolverPrivateLogic.h"

#include "ITimeManager.h"
#include "SingletonHolder.h"



bool Storm::SPHSolverPrivateLogic::shouldContinue() const
{
	const Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
	return timeMgr.isRunning();
}
