#pragma once

#include "EndPointIdentifier.h"


namespace Storm
{
	enum class NetworkMessageType;

	struct OnMessageReceivedParam
	{
	public:
		Storm::EndPointIdentifier _senderId;
		Storm::NetworkMessageType _messageType;
		std::vector<std::string> _parameters;
	};
}
