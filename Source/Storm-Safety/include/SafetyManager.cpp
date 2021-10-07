#include "SafetyManager.h"

#include "FreezeWatcher.h"
#include "MemoryWatcher.h"

#include "IThreadManager.h"
#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "ConfigConstants.h"

#include "ThreadHelper.h"
#include "ThreadFlagEnum.h"
#include "ThreadEnumeration.h"
#include "ThreadFlaggerObject.h"

#include "GeneralSafetyConfig.h"

#include "ThreadingSafety.h"


Storm::SafetyManager::SafetyManager() = default;

Storm::SafetyManager::~SafetyManager()
{
	{
		std::unique_lock<std::mutex> lock{ _cvMutex };
		_isRunning = false;
	}
	Storm::join(_safetyThread);

	_memoryWatcher.reset();
	_freezeWatcher.reset();
}

void Storm::SafetyManager::initialize_Implementation()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSafetyConfig &generalSafetyConfig = configMgr.getGeneralSafetyConfig();

	_freezeWatcher = std::make_unique<Storm::FreezeWatcher>(generalSafetyConfig);

	if (generalSafetyConfig._enableMemoryWatcher)
	{
		_memoryWatcher = std::make_unique<Storm::MemoryWatcher>(generalSafetyConfig);
	}

	_isRunning = true;

	_safetyThread = std::thread{ [this]()
	{
		STORM_REGISTER_THREAD(SafetyThread);
		STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::SafetyThread;
		this->run();
	} };
}

void Storm::SafetyManager::cleanUp_Implementation()
{
	{
		std::unique_lock<std::mutex> lock{ _cvMutex };
		_isRunning = false;
	}
	_cv.notify_all();

	Storm::join(_safetyThread);
}

void Storm::SafetyManager::run()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

	constexpr std::chrono::milliseconds k_refreshDurationMillisec{
		std::chrono::seconds{ static_cast<decltype(std::declval<std::chrono::seconds>().count())>(Storm::ConfigConstants::SafetyConstants::k_safetyThreadRefreshRateSeconds) / 5 }
	};

	std::unique_lock<std::mutex> lock{ _cvMutex };
	while (!_cv.wait_for(lock, k_refreshDurationMillisec, [this]() { return !_isRunning; }))
	{
		lock.unlock();

		threadMgr.processCurrentThreadActions();
		this->execute();

		lock.lock();
	}
}

void Storm::SafetyManager::execute()
{
	if (_memoryWatcher)
	{
		_memoryWatcher->execute();
	}
	_freezeWatcher->execute();
}

void Storm::SafetyManager::notifySimulationThreadAlive()
{
	assert(Storm::isSimulationThread() && "This method can only be executed inside Simulation thread.");

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::SafetyThread, [this, notificationTime = std::chrono::high_resolution_clock::now()]()
	{
		_freezeWatcher->setLastNotificationTime(notificationTime);
	});
}
