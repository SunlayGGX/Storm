#include "TcpClientBase.h"

#include "OnConnectionChangedParam.h"
#include "OnMessageReceivedParam.h"

#include "SocketSetting.h"

#include "StringAlgo.h"
#include "NetworkHelpers.h"
#include "ThreadingSafety.h"

#include "Network/NetworkConstants.cs"

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

	socket.async_connect(endpt, std::bind(&Storm::TCPClientBasePrivateLogic::connectHandler, this, std::placeholders::_1));
}

void Storm::TCPClientBasePrivateLogic::disconnect(Traits::NetworkService &ioService, Traits::SocketType &socket)
{
	socket.close();
}

void Storm::TCPClientBasePrivateLogic::autenthicate(Traits::SocketType &socket, Storm::EndPointIdentifier &cachedApplicationIdentifier, Storm::OnConnectionStateChangedParam &param)
{
	const auto remoteEndpoint = socket.remote_endpoint();
	cachedApplicationIdentifier._ipAddress = remoteEndpoint.address().to_v4().to_string();

	std::string autenthicationMsgToSend = Storm::NetworkHelpers::makeAuthenticationMsg();
	if (socket.send(boost::asio::buffer(autenthicationMsgToSend.data(), autenthicationMsgToSend.size())) != autenthicationMsgToSend.size())
	{
		Storm::throwException<Storm::Exception>("Wasn't able to authenticate ourself!");
	}

	std::size_t remainingSize =
		Storm::NetworkConstants::k_networkSeparator.size() * 4 +
		sizeof(Storm::NetworkConstants::k_magicKeyword) +
		sizeof(uint8_t) + // ApplicationType
		sizeof(uint32_t) + // PID
		sizeof(uint64_t) + // Version
		Storm::NetworkConstants::k_endOfMessageCommand.size()
		;

	std::string authenticatedMsgReceived;
	authenticatedMsgReceived.resize(remainingSize);

	do 
	{
		const std::size_t readByteCount = socket.read_some(boost::asio::buffer(authenticatedMsgReceived.data(), remainingSize));
		
		if (readByteCount > 0)
		{
			remainingSize -= readByteCount;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Broken socket while authenticating!");
		}

	} while (remainingSize != 0);

	std::vector<std::string_view> authenticatedTokens;
	authenticatedTokens.reserve(4);

	Storm::StringAlgo::split(authenticatedTokens, authenticatedMsgReceived, Storm::StringAlgo::makeSplitPredicate(Storm::NetworkConstants::k_networkSeparator));

	if (authenticatedTokens.size() == 5)
	{
		const uint32_t magicKeyword = Storm::NetworkHelpers::fromNetwork<uint32_t>(authenticatedTokens[0]);
		if (magicKeyword != Storm::NetworkConstants::k_magicKeyword)
		{
			Storm::throwException<Storm::Exception>("Wrong magic word received from connected application. Value received was " + std::to_string(magicKeyword));
		}

		if (authenticatedTokens[4] != Storm::NetworkConstants::k_endOfMessageCommand)
		{
			Storm::throwException<Storm::Exception>("Authentication message received from connected application did not finish by an end of message. Value received was " + Storm::toStdString(authenticatedTokens[4]));
		}

		cachedApplicationIdentifier._connectedApplication = static_cast<Storm::NetworkApplication>(Storm::NetworkHelpers::fromNetwork<uint8_t>(authenticatedTokens[1]));
		cachedApplicationIdentifier._pid = Storm::NetworkHelpers::fromNetwork<uint32_t>(authenticatedTokens[2]);
		
		param._applicationId = cachedApplicationIdentifier;
		param._applicationVersion = std::string{ Storm::toStdString(authenticatedTokens[3]).c_str() };

		param._connected = true;
	}
	else
	{
		Storm::throwException<Storm::Exception>("Authentication failed because of a wrong number of arguments!");
	}
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
	boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(_temporaryRead), Storm::NetworkConstants::k_endOfMessageCommand,
		[this, &socket](const boost::system::error_code &ec, const std::size_t byteRead)
	{
		if (!ec)
		{
			Storm::OnMessageReceivedParam param;
			if (Storm::NetworkHelpers::parseMsg(_temporaryRead, param))
			{
				this->notifyOnMessageReceived(std::move(param));
			}

			this->startRead(socket);
		}
		else
		{
			_temporaryRead.clear();
			this->disconnectLogicCall();
		}
	});
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

void Storm::TCPClientBasePrivateLogic::onConnectionChanged(const Storm::OnConnectionStateChangedParam &param, Storm::ConnectionStatus &connectedFlag)
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
	}
	else
	{
		connectedFlag = Storm::ConnectionStatus::NotConnected;
	}
}

void Storm::TCPClientBasePrivateLogic::onMessageReceived(const Storm::OnMessageReceivedParam &)
{
	assert(Storm::isNetworkThread() && "This method should only be called from Network thread!");
	_temporaryRead.clear();
}

