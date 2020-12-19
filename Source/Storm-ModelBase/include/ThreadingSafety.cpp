#include "ThreadingSafety.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"

#include "ThreadEnumeration.h"


namespace
{
	template<Storm::ThreadEnumeration threadEnum>
	__forceinline bool isExecutingOnRegisteredThread()
	{
		const Storm::IThreadManager &threadMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>();
		return threadMgr.isExecutingOnThread(threadEnum);
	}
}


bool Storm::isSimulationThread()
{
	return isExecutingOnRegisteredThread<Storm::ThreadEnumeration::MainThread>();
}

bool Storm::isLoadingThread()
{
	return isExecutingOnRegisteredThread<Storm::ThreadEnumeration::MainThread>();
}

bool Storm::isTimeThread()
{
	return isExecutingOnRegisteredThread<Storm::ThreadEnumeration::TimeThread>();
}

bool Storm::isGraphicThread()
{
	return isExecutingOnRegisteredThread<Storm::ThreadEnumeration::GraphicsThread>();
}

bool Storm::isSpaceThread()
{
	return isExecutingOnRegisteredThread<Storm::ThreadEnumeration::MainThread>();
}

bool Storm::isInputThread()
{
	return isExecutingOnRegisteredThread<Storm::ThreadEnumeration::WindowsAndInputThread>();
}

bool Storm::isLoggerThread()
{
	return isExecutingOnRegisteredThread<Storm::ThreadEnumeration::LoggerThread>();
}

bool Storm::isScriptThread()
{
	return isExecutingOnRegisteredThread<Storm::ThreadEnumeration::ScriptThread>();
}
