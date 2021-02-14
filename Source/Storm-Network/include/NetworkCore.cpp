#include "NetworkCore.h"

#include "ITCPClient.h"

#include "ITimeManager.h"
#include "SingletonHolder.h"


Storm::NetworkCore::~NetworkCore() = default;

void Storm::NetworkCore::initialize()
{

}

void Storm::NetworkCore::execute()
{
	for (const auto &client : _clients)
	{
		if (!client->isConnected())
		{
			client->connect();
		}
	}

	Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();

	constexpr std::chrono::milliseconds totalRunTime{ 12 };
	constexpr std::chrono::milliseconds iterationRunTime{ 4 };

	const auto quitTime = std::chrono::high_resolution_clock::now() + totalRunTime;
	while (timeMgr.isRunning())
	{
		_networkService.run_for(iterationRunTime);

		if (std::chrono::high_resolution_clock::now() > quitTime)
		{
			break;
		}
	}
}

void Storm::NetworkCore::destroyTCPClient(Storm::ITCPClient*const client)
{
	client->close();
	delete client;
}
