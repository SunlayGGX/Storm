#include "NetworkScriptReceiver.h"

#include "SingletonHolder.h"
#include "IScriptManager.h"
#include "IConfigManager.h"
#include "NetworkManager.h"

#include "SocketSetting.h"
#include "GeneralNetworkConfig.h"

#include "EndPointIdentifier.h"

#include "OnMessageReceivedParam.h"
#include "OnConnectionChangedParam.h"
#include "NetworkSendParam.h"

#include "ThreadingSafety.h"

#include "Network/NetworkApplication.cs"
#include "Network/NetworkMessageType.cs"



Storm::NetworkScriptReceiver::NetworkScriptReceiver(ParentTcpClientBase::Traits::NetworkService &ioService) :
	Storm::NetworkScriptReceiver::ParentTcpClientBase{ ioService }
{

}

void Storm::NetworkScriptReceiver::send(Storm::NetworkSendParam &&/*sendParam*/)
{
	STORM_NOT_IMPLEMENTED;
}

void Storm::NetworkScriptReceiver::onConnectionChanged_Implementation(Storm::OnConnectionStateChangedParam &&param)
{
	if (param._connected)
	{
		if (param._applicationId._connectedApplication != Storm::NetworkApplication::Storm_ScriptSender)
		{
			Storm::throwException<Storm::Exception>("Connected application is not one to receive script from!");
		}

		LOG_DEBUG << "Script sender from " << param._applicationId._ipAddress << " connected.";
	}
	else
	{
		LOG_DEBUG << "Script sender from " << param._applicationId._ipAddress << " disconnected.";
	}

	Storm::NetworkManager &networkMgr = Storm::NetworkManager::instance();
	networkMgr.notifyApplicationConnectionChanged(std::move(param));
}

void Storm::NetworkScriptReceiver::onMessageReceived_Implementation(Storm::OnMessageReceivedParam &&receivedMsg)
{
	assert(Storm::isNetworkThread() && "This method should only be called from Network thread!");

	if (receivedMsg._messageType == Storm::NetworkMessageType::Script)
	{
		if (receivedMsg._senderId._connectedApplication == Storm::NetworkApplication::Storm_ScriptSender)
		{
			const std::size_t argumentCount = receivedMsg._parameters.size();
			if (argumentCount == 1)
			{
				Storm::IScriptManager &scriptMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IScriptManager>();
				scriptMgr.execute(std::move(receivedMsg._parameters[0]));
			}
			else
			{
				LOG_ERROR << "Wrong number of arguments, ignoring! Arguments were :" << receivedMsg._parameters;
			}
		}
		else
		{
			LOG_ERROR << "Message received from an application that should not send scripts. Ignoring :" << receivedMsg._parameters;
		}
	}
	else
	{
		LOG_ERROR << "Message isn't a script!";
	}
}

const Storm::SocketSetting& Storm::NetworkScriptReceiver::getUsedSocketSetting_Implementation()
{
	return *Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getGeneralNetworkConfig()._scriptSenderSocket;
}

bool Storm::NetworkScriptReceiver::shouldCreate()
{
	const Storm::SocketSetting &currentSetting = Storm::NetworkScriptReceiver::getUsedSocketSetting_Implementation();
	return currentSetting._isEnabled && currentSetting.isValid();
}
