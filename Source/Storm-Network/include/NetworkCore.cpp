#include "NetworkCore.h"

#include "ITCPClient.h"
#include "NetworkScriptReceiver.h"

#include "ITimeManager.h"
#include "SingletonHolder.h"

#include "ThreadingSafety.h"


Storm::NetworkCore::NetworkCore()
{
	assert(Storm::isMainThread() && "NetworkCore construction should happen in the main thread!");
}

Storm::NetworkCore::~NetworkCore()
{
	assert(Storm::isMainThread() && "NetworkCore destruction should happen in the main thread!");
}

bool Storm::NetworkCore::initialize()
{
	if (Storm::NetworkScriptReceiver::shouldCreate())
	{
		_clients.emplace_back(TcpClientSessionPtr{ new Storm::NetworkScriptReceiver{ _networkService }, &Storm::NetworkCore::destroyTCPClient });
	}

	return !_clients.empty();
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
		try
		{
			_networkService.run_for(iterationRunTime);
		}
		catch (const Storm::Exception &exception)
		{
			LOG_ERROR <<
				"Network error happened :\n"
				"Error: " << exception.what() << ".\n"
				"at " << exception.stackTrace();
		}

		if (std::chrono::high_resolution_clock::now() > quitTime)
		{
			break;
		}
	}
}

void Storm::NetworkCore::close()
{
	// We never add clients after the initialization, therefore since the vector doesn't change, no need to lock anything... This is thread safe.
	for (const auto &client : _clients)
	{
		client->definitiveStop();
	}

	_networkService.stop();
}

void Storm::NetworkCore::destroyTCPClient(Storm::ITCPClient*const client)
{
	client->close();
	delete client;
}
