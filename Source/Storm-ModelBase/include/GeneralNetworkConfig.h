#pragma once


namespace Storm
{
	struct SocketSetting;

	struct GeneralNetworkConfig
	{
	public:
		GeneralNetworkConfig();
		~GeneralNetworkConfig();

	public:
		bool _enableNetwork;
		std::unique_ptr<Storm::SocketSetting> _scriptSenderSocket;
	};
}
