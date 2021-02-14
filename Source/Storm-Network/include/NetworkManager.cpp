#include "NetworkManager.h"

#include "NetworkCore.h"

#include "IThreadManager.h"
#include "ITimeManager.h"
#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "GeneralNetworkConfig.h"

#include "ThreadFlagEnum.h"
#include "ThreadFlaggerObject.h"
#include "ThreadEnumeration.h"
#include "ThreadHelper.h"
#include "ThreadingSafety.h"

#include "StormExiter.h"

Storm::NetworkManager::NetworkManager() = default;
Storm::NetworkManager::~NetworkManager() = default;

void Storm::NetworkManager::initialize_Implementation()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	if (configMgr.getGeneralNetworkConfig()._enableNetwork)
	{
		_netCore = std::make_unique<Storm::NetworkCore>();

		_networkThread = std::thread{ [this]()
		{
			STORM_REGISTER_THREAD(NetworkThread);
			STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::NetworkThread;

			this->run();
		} };
	}
	else
	{
		LOG_WARNING << "Network disabled. The associated thread would only be a dummy!";
		
		_networkThread = std::thread{ [this]()
		{
			STORM_REGISTER_THREAD(NetworkThread);
			STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::NetworkThread;

			this->dummyNoRun();
		} };
	}
}

void Storm::NetworkManager::run()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

	try
	{
		_netCore->initialize();

		constexpr std::chrono::milliseconds k_refreshMillisec{ 64 };
		while (timeMgr.waitForTimeOrExit(k_refreshMillisec))
		{
			threadMgr.processCurrentThreadActions();
			_netCore->execute();
		}
	}
	catch (const Storm::Exception &e)
	{
		LOG_FATAL << 
			"Storm::Exception catched forcing us to stop the network module.\n"
			"Error was " << e.what() << ".\n"
			"Call stack was " << e.stackTrace()
			;

		Storm::requestExitOtherThread();
	}
	catch (const std::exception &e)
	{
		LOG_FATAL <<
			"std::exception catched forcing us to stop the network module.\n"
			"Error was " << e.what() << "."
			;

		Storm::requestExitOtherThread();
	}
	catch (...)
	{
		LOG_FATAL <<
			"unknown exception catched forcing us to stop the network module."
			;

		Storm::requestExitOtherThread();
	}

	_netCore.reset();
}

void Storm::NetworkManager::dummyNoRun()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();

	constexpr std::chrono::seconds k_refreshMillisec{ 3 };
	while (timeMgr.waitForTimeOrExit(k_refreshMillisec))
	{
		// Just clear to not accumulate actions we won't process.
		threadMgr.clearCurrentThreadActions();
	}
}

void Storm::NetworkManager::cleanUp_Implementation()
{
	Storm::join(_networkThread);
}
