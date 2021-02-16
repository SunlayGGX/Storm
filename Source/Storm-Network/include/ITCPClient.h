#pragma once


namespace Storm
{
	struct OnConnectionStateChangedParam;
	struct OnMessageReceivedParam;
	struct NetworkSendParam;

	struct __declspec(novtable) ITCPClient
	{
	public:
		virtual ~ITCPClient() = default;

	public:
		virtual void close() = 0;

		virtual void definitiveStop() = 0;

	public:
		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual bool isConnected() const = 0;
		virtual bool shouldConnect() const = 0;

	public:
		virtual void send(Storm::NetworkSendParam &&sendParam) = 0;

	public:
		virtual void onConnectionChanged(Storm::OnConnectionStateChangedParam &&param) = 0;
		virtual void onMessageReceived(Storm::OnMessageReceivedParam &&receivedMsg) = 0;

	};
}
