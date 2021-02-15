#pragma once


namespace Storm
{
	struct ITCPClient;

	class TcpClientHandler
	{
	public:
		TcpClientHandler();

	public:
		void execute();

	private:
		std::unique_ptr<Storm::ITCPClient> _client;
	};
}
