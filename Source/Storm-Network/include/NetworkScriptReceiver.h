#pragma once

#include "TCPClientBase.h"


namespace Storm
{
	class NetworkScriptReceiver : public Storm::TCPClientBase<Storm::NetworkScriptReceiver>
	{
	public:
		NetworkScriptReceiver(ParentTcpClientBase::Traits::NetworkService &ioService);

	public:
		void send(Storm::NetworkSendParam &&sendParam) final override;

	public:
		void onConnectionChanged_Implementation(Storm::OnConnectionStateChangedParam &&param);
		void onMessageReceived_Implementation(Storm::OnMessageReceivedParam &&receivedMsg);

		static const Storm::SocketSetting& getUsedSocketSetting_Implementation();

	public:
		static bool shouldCreate();

	private:
		STORM_DECLARE_TCPClientBase_BODY();
	};
}
