#pragma once

#include "Singleton.h"
#include "INetworkManager.h"


namespace Storm
{
	class NetworkCore;
	class NetworkManager final :
		private Storm::Singleton<Storm::NetworkManager>,
		public Storm::INetworkManager
	{
		STORM_DECLARE_SINGLETON(NetworkManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	private:
		void run();
		void dummyNoRun();

	private:
		std::unique_ptr<Storm::NetworkCore> _netCore;

		std::thread _networkThread;
	};
}
