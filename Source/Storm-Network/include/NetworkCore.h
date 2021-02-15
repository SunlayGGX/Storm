#pragma once

#include <boost\asio\io_service.hpp>


namespace Storm
{
	struct ITCPClient;

	class NetworkCore
	{
	public:
		~NetworkCore();

	public:
		bool initialize();

	public:
		void execute();

	private:
		static void destroyTCPClient(Storm::ITCPClient*const client);

	public:
		using TcpClientSessionPtr = std::unique_ptr<Storm::ITCPClient, decltype(&Storm::NetworkCore::destroyTCPClient)>;

	private:
		boost::asio::io_service _networkService;
		std::vector<TcpClientSessionPtr> _clients;
	};
}
