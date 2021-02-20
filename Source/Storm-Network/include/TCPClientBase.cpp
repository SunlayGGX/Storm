#include "TcpClientBase.h"

#include "OnConnectionChangedParam.h"
#include "OnMessageReceivedParam.h"

#include "SocketSetting.h"

#include "StringAlgo.h"
#include "NetworkHelpers.h"
#include "ThreadingSafety.h"
#include "RAII.h"

#include "Network/NetworkConstants.cs"
#include "Network/NetworkMessageType.cs"
#include "Network/NetworkApplication.cs"

#include <boost/asio/read_until.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/address.hpp>




void Storm::TCPClientBasePrivateLogic::setup(Traits::SocketType &socket)
{
	/*socket.set_option(boost::asio::socket_base::keep_alive{ true });
	socket.set_option(boost::asio::socket_base::reuse_address{ true });*/
}

void Storm::TCPClientBasePrivateLogic::connect(Traits::NetworkService &ioService, Traits::SocketType &socket, const Storm::SocketSetting &settings)
{
	Traits::TcpType::endpoint endpt{
		boost::asio::ip::address::from_string(settings.getIPStr()),
		settings._port
	};

	socket.async_connect(endpt, [this, &socket](const boost::system::error_code &ec)
	{
		if (!ec)
		{
			try
			{
				this->notifyStartAuthentication();

				this->doAuthentication(socket);
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
		}

		this->disconnectLogicCall();
	});
}

void Storm::TCPClientBasePrivateLogic::disconnect(Traits::NetworkService &ioService, Traits::SocketType &socket)
{
	socket.close();
}

void Storm::TCPClientBasePrivateLogic::doAuthentication(Traits::SocketType &socket)
{
	Storm::EndPointIdentifier applicationIdentifier;

	const auto remoteEndpoint = socket.remote_endpoint();
	applicationIdentifier._ipAddress = remoteEndpoint.address().to_v4().to_string();

	_temporaryRead.clear();
	boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(_temporaryRead), Storm::NetworkConstants::k_endOfMessageCommand,
		[this, appIdentifier = std::move(applicationIdentifier)](const boost::system::error_code &ec, const std::size_t) mutable
	{
		auto disconnectRaiiObject = Storm::makeLazyRAIIObject([this]() 
		{
			_temporaryRead.clear();
			this->disconnectLogicCall();
		});

		if (ec)
		{
			return;
		}

		std::string authenticatedMsgReceived = std::move(_temporaryRead);

		const std::size_t authenticateSize =
			Storm::NetworkConstants::k_networkSeparator.size() * 4 +
			sizeof(Storm::NetworkConstants::k_magicKeyword) +
			sizeof(uint8_t) + // ApplicationType
			sizeof(uint32_t) + // PID
			sizeof(uint64_t) + // Version
			Storm::NetworkConstants::k_endOfMessageCommand.size()
			;

		if (authenticatedMsgReceived.size() != authenticateSize)
		{
			LOG_ERROR << "Wrong authentication message size received from " << appIdentifier._ipAddress << "!";
			return;
		}

		std::vector<std::string_view> authenticatedTokens;

		Storm::StringAlgo::split(authenticatedTokens, authenticatedMsgReceived, Storm::StringAlgo::makeSplitPredicate(Storm::NetworkConstants::k_networkSeparator));

		if (authenticatedTokens.size() == 5)
		{
			const uint32_t magicKeyword = Storm::NetworkHelpers::fromNetwork<uint32_t>(authenticatedTokens[0]);
			if (magicKeyword != Storm::NetworkConstants::k_magicKeyword)
			{
				LOG_ERROR << "Wrong magic word received received from " << appIdentifier._ipAddress << ". Value received was " << magicKeyword;
				return;
			}

			if (authenticatedTokens[4] != Storm::NetworkConstants::k_endOfMessageCommand)
			{
				LOG_ERROR << "Authentication message received from received from " << appIdentifier._ipAddress << " did not finish by an end of message. Value received was " << authenticatedTokens[4];
				return;
			}

			appIdentifier._connectedApplication = static_cast<Storm::NetworkApplication>(Storm::NetworkHelpers::fromNetwork<uint8_t>(authenticatedTokens[1]));
			appIdentifier._pid = Storm::NetworkHelpers::fromNetwork<uint32_t>(authenticatedTokens[2]);

			if (authenticatedTokens[3].empty())
			{
				LOG_ERROR << "We didn't received a network version. The authentication message received from " << appIdentifier._ipAddress << " is invalid!";
				return;
			}

			Storm::OnConnectionStateChangedParam param;
			param._applicationId = appIdentifier;

			std::size_t versionSize = 0;
			for (; authenticatedTokens[3][versionSize] != '\0'; ++versionSize);
			param._applicationVersion = std::string{ authenticatedTokens[3].data(), versionSize };

			param._connected = true;

			this->notifyOnConnectionChanged(std::move(param));

			// The only way to not disconnect is to successfully come here.
			_temporaryRead.clear();
			disconnectRaiiObject.release();
		}
		else
		{
			LOG_ERROR << "Authentication failed because of a wrong number of arguments received from " << appIdentifier._ipAddress << "!";
		}
	});

	std::string authenticationMsgToSend = Storm::NetworkHelpers::makeAuthenticationMsg();
	const std::size_t authenticationMsgLength = authenticationMsgToSend.size();
	auto buffer = boost::asio::buffer(authenticationMsgToSend.data(), authenticationMsgLength);
	socket.async_send(buffer, [this, aliveKeeper = std::move(authenticationMsgToSend), authenticationMsgLength](const boost::system::error_code &ec, const std::size_t byteWrote)
	{
		if (!ec && byteWrote == authenticationMsgLength)
		{
			LOG_DEBUG << "Authenticated ourself successfully!";
		}
		else
		{
			LOG_ERROR << "We weren't able to authenticate ourself!";
			this->disconnectLogicCall();
		}
	});
}

void Storm::TCPClientBasePrivateLogic::sendMessage(Traits::SocketType &socket, std::string &&msg, const Storm::NetworkMessageType messageType)
{
	Storm::NetworkHelpers::prepareMsg(msg, messageType);
	socket.send(boost::asio::buffer(msg.data(), msg.size()));
}

void Storm::TCPClientBasePrivateLogic::sendAsyncMessage(Traits::SocketType &socket, std::string &&msg, const Storm::NetworkMessageType messageType, std::size_t byteWroteSince /*= 0*/)
{
	if (byteWroteSince == 0)
	{
		Storm::NetworkHelpers::prepareMsg(msg, messageType);
	}

	const auto sendBuffer = boost::asio::buffer(msg.data() + byteWroteSince, msg.size());

	socket.async_send(
		sendBuffer,
		[this, &socket, messageType, currentMsg = std::move(msg), byteWroteSince](const boost::system::error_code &ec, const std::size_t byteWrote) mutable
	{
		if (!ec)
		{
			byteWroteSince += byteWrote;
			if (byteWroteSince != currentMsg.size())
			{
				this->sendAsyncMessage(socket, std::move(currentMsg), messageType, byteWroteSince);
			}
		}
		else
		{
			this->disconnectLogicCall();
		}
	});
}

void Storm::TCPClientBasePrivateLogic::startRead(Traits::SocketType &socket)
{
	if (this->queryIsConnected())
	{
		boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(_temporaryRead), Storm::NetworkConstants::k_endOfMessageCommand,
			[this, &socket](const boost::system::error_code &ec, const std::size_t byteRead)
		{
			if (!ec)
			{
				auto raiiRestartRead = Storm::makeLazyRAIIObject([this, &socket]()
				{
					this->startRead(socket);
				});

				std::vector<Storm::OnMessageReceivedParam> params;
				if (Storm::NetworkHelpers::parseMsg(_temporaryRead, params))
				{
					for (Storm::OnMessageReceivedParam &param : params)
					{
						this->notifyOnMessageReceived(std::move(param));
					}
				}
			}
			else
			{
				_temporaryRead.clear();
				this->disconnectLogicCall();
			}
		});
	}
}

void Storm::TCPClientBasePrivateLogic::definitiveStop(Traits::SocketType &socket, const bool connected)
{
	// This is thread safe, so no need to lock anything.
	socket.cancel();
	if (connected)
	{
		socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both);
	}
	socket.close();
}

void Storm::TCPClientBasePrivateLogic::onConnectionChanged(const Storm::OnConnectionStateChangedParam &param, Storm::ConnectionStatus &connectedFlag, Storm::EndPointIdentifier &outApplicationId)
{
	assert(Storm::isNetworkThread() && "This method should only be called from Network thread!");

	if (param._connected)
	{
		connectedFlag = Storm::ConnectionStatus::Connected;
		if (param._applicationVersion != Storm::NetworkConstants::k_networkVersion)
		{
			Storm::throwException<Storm::Exception>(
				"Application's version to connect with is mismatching with our own.\n"
				"Ours : " + Storm::toStdString(Storm::NetworkConstants::k_networkVersion) + "."
				"Theirs : " + Storm::toStdString(param._applicationVersion) + ".\n"
				"We cannot accept the connection until both versions are the same!"
			);
		}

		outApplicationId = param._applicationId;
	}
	else
	{
		connectedFlag = Storm::ConnectionStatus::NotConnected;

		outApplicationId._connectedApplication = Storm::NetworkApplication::Unknown;
		outApplicationId._ipAddress.clear();
		outApplicationId._pid = 0;
	}
}

bool Storm::TCPClientBasePrivateLogic::onMessageReceived(const Storm::OnMessageReceivedParam &param)
{
	assert(Storm::isNetworkThread() && "This method should only be called from Network thread!");
	_temporaryRead.clear();

	return param._messageType != Storm::NetworkMessageType::Ping;
}

