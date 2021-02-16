#pragma once

#include "ITCPClient.h"

#include "MethodEnsurerMacro.h"
#include "EndPointIdentifier.h"

#include <boost\asio\io_service.hpp>
#include <boost\asio\ip\tcp.hpp>


namespace Storm
{
	struct OnConnectionStateChangedParam;
	struct OnMessageReceivedParam;
	struct SocketSetting;
	enum class NetworkMessageType;

	enum class ConnectionStatus
	{
		NotConnected,
		Connecting,
		WaitingForAuthentication,
		Connected,
	};

	struct TCPClientBaseTraits
	{
	public:
		using TcpType = boost::asio::ip::tcp;
		using SocketType = TcpType::socket;
		using NetworkService = boost::asio::io_service;
	};

	class TCPClientBasePrivateLogic
	{
	public:
		using Traits = Storm::TCPClientBaseTraits;

	protected:
		void setup(Traits::SocketType &socket);

	protected:
		void connect(Traits::NetworkService &ioService, Traits::SocketType &socket, const Storm::SocketSetting &settings);
		void disconnect(Traits::NetworkService &ioService, Traits::SocketType &socket);

		void sendMessage(Traits::SocketType &socket, std::string &&msg, const Storm::NetworkMessageType messageType);
		void sendAsyncMessage(Traits::SocketType &socket, std::string &&msg, const Storm::NetworkMessageType messageType, std::size_t byteWroteSince = 0);

		void startRead(Traits::SocketType &socket);

		void definitiveStop(Traits::SocketType &socket, const bool connected);

	private:
		void doAuthentication(Traits::SocketType &socket);

	protected:
		void onConnectionChanged(const Storm::OnConnectionStateChangedParam &param, Storm::ConnectionStatus &connectedFlag, Storm::EndPointIdentifier &outApplicationId);
		bool onMessageReceived(const Storm::OnMessageReceivedParam &param);

	protected:
		// Method to trigger logic from the base
		virtual void disconnectLogicCall() = 0;
		virtual void notifyStartAuthentication() = 0;
		virtual void notifyOnConnectionChanged(Storm::OnConnectionStateChangedParam &&param) = 0;
		virtual void notifyOnMessageReceived(Storm::OnMessageReceivedParam &&param) = 0;

	private:
		std::string _temporaryRead;
	};

	template<class Child>
	class TCPClientBase :
		public Storm::ITCPClient,
		private Storm::TCPClientBasePrivateLogic
	{
	protected:
		using ParentTcpClientBase = Storm::TCPClientBase<Child>;
		using Traits = Storm::TCPClientBasePrivateLogic::Traits;

	private:
		using Logic = Storm::TCPClientBasePrivateLogic;

	protected:
		// Workaround to CRTP.
		template<int> struct Checker
		{
			STORM_ENSURE_HAS_METHOD(Child, void, onConnectionChanged_Implementation, Storm::OnConnectionStateChangedParam);
			STORM_ENSURE_HAS_METHOD(Child, void, onMessageReceived_Implementation, Storm::OnMessageReceivedParam);
			STORM_ENSURE_HAS_METHOD(Child, const Storm::SocketSetting&, getUsedSocketSetting_Implementation);

			enum{ dummyBody };
		};

	public:
		TCPClientBase(Traits::NetworkService &ioService) :
			_status{ Storm::ConnectionStatus::NotConnected },
			_netService{ &ioService },
			_socket{ ioService }
		{
			Logic::setup(_socket);
		}

		virtual ~TCPClientBase() = default;

	public:
		void close() final override
		{
			this->disconnect();
			_netService = nullptr;
		}

	public:
		void connect() final override
		{
			if (_status == Storm::ConnectionStatus::NotConnected)
			{
				_status = Storm::ConnectionStatus::Connecting;

				const Child &currentNoIndirect = static_cast<const Child&>(*this);
				Logic::connect(*_netService, _socket, currentNoIndirect.getUsedSocketSetting_Implementation());
			}
		}

		void disconnect() final override
		{
			if (_status == Storm::ConnectionStatus::Connected)
			{
				Logic::disconnect(*_netService, _socket);
			}

			_status = Storm::ConnectionStatus::NotConnected;
		}

		bool isConnected() const noexcept final override
		{
			return _status == Storm::ConnectionStatus::Connected || _status == Storm::ConnectionStatus::WaitingForAuthentication;
		}

		bool shouldConnect() const noexcept final override
		{
			return _status == Storm::ConnectionStatus::NotConnected;
		}

	public:
		void definitiveStop() final override
		{
			Logic::definitiveStop(_socket, this->isConnected());
		}

	public:
		void onConnectionChanged(Storm::OnConnectionStateChangedParam &&param) final override
		{
			Logic::onConnectionChanged(param, _status, _cachedApplicationIdentifier);

			Child &currentNoIndirect = static_cast<Child&>(*this);
			currentNoIndirect.onConnectionChanged_Implementation(std::move(param));

			if (this->isConnected())
			{
				Logic::startRead(_socket);
			}
		}

		void onMessageReceived(Storm::OnMessageReceivedParam &&receivedMsg) final override
		{
			if (Logic::onMessageReceived(receivedMsg))
			{
				Child &currentNoIndirect = static_cast<Child&>(*this);
				currentNoIndirect.onMessageReceived_Implementation(std::move(receivedMsg));
			}
		}

	private:
		void notifyStartAuthentication() final override
		{
			_status = Storm::ConnectionStatus::WaitingForAuthentication;
		}

		void notifyOnConnectionChanged(Storm::OnConnectionStateChangedParam &&param) final override
		{
			this->onConnectionChanged(std::move(param));
		}

		void disconnectLogicCall() final override
		{
			Logic::disconnect(*_netService, _socket);
			this->disconnect();
		}

		void notifyOnMessageReceived(Storm::OnMessageReceivedParam &&param) final override
		{
			this->onMessageReceived(std::move(param));
		}

	protected:
		Storm::ConnectionStatus _status;
		Traits::NetworkService* _netService;
		Traits::SocketType _socket;
		Storm::EndPointIdentifier _cachedApplicationIdentifier;
	};

	// Use this macro at the end of your derived class.
#define STORM_DECLARE_TCPClientBase_BODY() enum { STORM_CONCAT(___unuse__, __LINE__) = ParentTcpClientBase::Checker<0>::dummyBody }
}
