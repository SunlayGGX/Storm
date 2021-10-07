#include "SafetyManager.h"

#include "FreezeWatcher.h"
#include "MemoryWatcher.h"

#include "IThreadManager.h"
#include "ITimeManager.h"
#include "SingletonHolder.h"

#include "ConfigConstants.h"

#include "ThreadHelper.h"
#include "ThreadFlagEnum.h"
#include "ThreadEnumeration.h"
#include "ThreadFlaggerObject.h"


Storm::SafetyManager::SafetyManager() = default;

Storm::SafetyManager::~SafetyManager()
{
	Storm::join(_safetyThread);

	_memoryWatcher.reset();
	_freezeWatcher.reset();
}

void Storm::SafetyManager::initialize_Implementation()
{
	_freezeWatcher = std::make_unique<Storm::FreezeWatcher>();
	_memoryWatcher = std::make_unique<Storm::MemoryWatcher>();

	_safetyThread = std::thread{ [this]()
	{
		STORM_REGISTER_THREAD(SafetyThread);
		STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::SafetyThread;
		this->run();
	} };
}

void Storm::SafetyManager::cleanUp_Implementation()
{
	Storm::join(_safetyThread);
}

void Storm::SafetyManager::run()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

	constexpr std::chrono::milliseconds k_refreshDurationMillisec{
		std::chrono::seconds{ static_cast<decltype(std::declval<std::chrono::seconds>().count())>(Storm::ConfigConstants::SafetyConstants::k_safetyThreadRefreshRateSeconds) }
	};

	while (!timeMgr.waitForTimeOrExit(k_refreshDurationMillisec))
	{
		threadMgr.processCurrentThreadActions();
		this->execute();
	}
}

void Storm::SafetyManager::execute()
{

}
