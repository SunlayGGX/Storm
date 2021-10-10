#include "NetworkManager.h"

#include "NetworkCore.h"

#include "IThreadManager.h"
#include "ITimeManager.h"
#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "GeneralNetworkConfig.h"

#include "OnConnectionChangedParam.h"

#include "ThreadFlagEnum.h"
#include "ThreadFlaggerObject.h"
#include "ThreadEnumeration.h"
#include "ThreadHelper.h"
#include "ThreadingSafety.h"

#include "StormExiter.h"

#include <boost/algorithm/string/case_conv.hpp>


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

	bool runContinue;

	try
	{
		runContinue = _netCore->initialize();
		if (runContinue)
		{
			constexpr std::chrono::milliseconds k_refreshMillisec{ 64 };
			while (timeMgr.waitForTimeOrExit(k_refreshMillisec))
			{
				threadMgr.processCurrentThreadActions();
				_netCore->execute();
			}

			runContinue = false;
		}
		else
		{
			// To dummy run.
			runContinue = true;
		}
	}
	catch (const Storm::Exception &e)
	{
		LOG_FATAL << 
			"Storm::Exception catched forcing us to stop the network module.\n"
			"Error was " << e.what() << ".\n"
			"Stack trace :\n" << e.stackTrace()
			;
		
		runContinue = false;
		Storm::requestExitOtherThread();
	}
	catch (const std::exception &e)
	{
		LOG_FATAL <<
			"std::exception catched forcing us to stop the network module.\n"
			"Error was " << e.what() << "."
			;

		runContinue = false;
		Storm::requestExitOtherThread();
	}
	catch (...)
	{
		LOG_FATAL <<
			"unknown exception catched forcing us to stop the network module."
			;

		runContinue = false;
		Storm::requestExitOtherThread();
	}

	if (runContinue)
	{
		this->dummyNoRun();
	}
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
	if (_netCore != nullptr)
	{
		_netCore->close();
	}

	Storm::join(_networkThread);
	_netCore.reset();
}

void Storm::NetworkManager::notifyApplicationConnectionChanged(Storm::OnConnectionStateChangedParam &&param)
{
	assert(Storm::isNetworkThread() && "This method should only be called from Network thread!");

	boost::algorithm::to_lower(param._applicationId._ipAddress);

	const std::string uniqueIpAddress =
		(param._applicationId._ipAddress == "localhost") ? "127.0.0.1" : std::move(param._applicationId._ipAddress)
		;

	if (param._connected)
	{
		_connectedPIDs[uniqueIpAddress].emplace_back(param._applicationId._pid);
	}
	else if(auto found = _connectedPIDs.find(uniqueIpAddress); found != std::end(_connectedPIDs))
	{
		bool pidWasRegistered = false;

		std::vector<unsigned int> &connectedPids = found->second;
		const std::size_t connectedCount = connectedPids.size();
		for (std::size_t iter = 0; iter < connectedCount; ++iter)
		{
			if (connectedPids[iter] == param._applicationId._pid)
			{
				if (iter != connectedCount - 1)
				{
					std::swap(connectedPids[iter], connectedPids.back());
				}

				connectedPids.pop_back();

				pidWasRegistered = true;
				break;
			}
		}

		assert(pidWasRegistered && "PID wasn't registered! Maybe we forgot to notify a connection!");
	}
}
