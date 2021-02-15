#pragma once

#include "ITCPClient.h"

#include "MethodEnsurerMacro.h"

#include "OnConnectionChangedParam.h"

#include <boost\asio\io_service.hpp>
#include <boost\asio\ip\tcp.hpp>


namespace Storm
{
	struct OnMessageReceivedParam;
	struct SocketSetting;
	enum class NetworkMessageType;

	enum class ConnectionStatus
	{
		NotConnected,
		Connecting,
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

		void autenthicate(Traits::SocketType &socket, Storm::EndPointIdentifier &cachedApplicationIdentifier, Storm::OnConnectionStateChangedParam &param);

		void sendMessage(Traits::SocketType &socket, std::string &&msg, const Storm::NetworkMessageType messageType);
		void sendAsyncMessage(Traits::SocketType &socket, std::string &&msg, const Storm::NetworkMessageType messageType, std::size_t byteWroteSince = 0);

		void startRead(Traits::SocketType &socket);

	protected:
		void onConnectionChanged(const Storm::OnConnectionStateChangedParam &param, Storm::ConnectionStatus &connectedFlag);
		void onMessageReceived(const Storm::OnMessageReceivedParam &param);

	protected:
		// Method to trigger logic from the base
		virtual void connectHandler(const boost::system::error_code &ec) = 0;
		virtual void disconnectLogicCall() = 0;
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
			return _status == Storm::ConnectionStatus::Connected;
		}

		bool shouldConnect() const noexcept final override
		{
			return _status == Storm::ConnectionStatus::NotConnected;
		}

	public:
		void onConnectionChanged(Storm::OnConnectionStateChangedParam &&param) final override
		{
			Child &currentNoIndirect = static_cast<Child&>(*this);

			try
			{
				Logic::onConnectionChanged(param, _status);
				currentNoIndirect.onConnectionChanged_Implementation(std::move(param));

				if (this->isConnected())
				{
					Logic::startRead(_socket);
				}
			}
			catch (...)
			{
				// If we catch something here, then it is considered as a connection refusal from our end. So we disconnect.

				if (this->isConnected())
				{
					this->disconnect();
				}
				throw;
			}
		}

		void onMessageReceived(Storm::OnMessageReceivedParam &&receivedMsg) final override
		{
			Logic::onMessageReceived(receivedMsg);
			
			Child &currentNoIndirect = static_cast<Child&>(*this);
			currentNoIndirect.onMessageReceived_Implementation(std::move(receivedMsg));
		}

	private:
		void connectHandler(const boost::system::error_code &ec) final override
		{
			if (!ec)
			{
				Child &currentNoIndirect = static_cast<Child&>(*this);

				try
				{
					Storm::OnConnectionStateChangedParam param;
					Logic::autenthicate(_socket, _cachedApplicationIdentifier, param);

					this->onConnectionChanged(std::move(param));

					return;
				}
				catch (const Storm::Exception &e)
				{
					LOG_ERROR <<
						"Storm::Exception catched while connecting : " << e.what() << ".\n"
						"Stack trace :\n" << e.stackTrace()
						;
				}
				catch (const std::exception &e)
				{
					LOG_ERROR << "std::exception catched while connecting : " << e.what();
				}

				this->disconnect();
			}
			else
			{
				this->disconnect();
			}
		}

		void disconnectLogicCall() final override
		{
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
