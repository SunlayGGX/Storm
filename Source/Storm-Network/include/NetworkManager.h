#pragma once

#include "Singleton.h"
#include "INetworkManager.h"


namespace Storm
{
	struct OnConnectionStateChangedParam;
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

	public:
		void notifyApplicationConnectionChanged(Storm::OnConnectionStateChangedParam &&param);

	private:
		std::map<std::string, std::vector<unsigned int>> _connectedPIDs;
		std::unique_ptr<Storm::NetworkCore> _netCore;

		std::thread _networkThread;
	};
}
