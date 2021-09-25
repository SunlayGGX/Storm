#include "NetworkHelpers.h"

#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "OnMessageReceivedParam.h"

#include "StringAlgo.h"
#include "RAII.h"

#include "Network/NetworkConstants.cs"
#include "Network/NetworkApplication.cs"


void Storm::NetworkHelpers::prepareMsg(std::string &inOutMsg, const Storm::NetworkMessageType messageType)
{
	uint32_t pid = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getCurrentPID();

	constexpr std::size_t additionalSize =
		Storm::NetworkConstants::k_networkSeparator.size() * 5 +
		sizeof(Storm::NetworkConstants::k_magicKeyword) +
		sizeof(uint8_t) + // ApplicationType
		sizeof(uint32_t) + // PID
		sizeof(uint8_t) + // MessageType
		Storm::NetworkConstants::k_endOfMessageCommand.size()
		;

	std::string msgWithHeader;
	msgWithHeader.reserve(inOutMsg.size() + additionalSize);

	msgWithHeader += Storm::NetworkHelpers::toNetwork(Storm::NetworkConstants::k_magicKeyword);
	
	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += static_cast<const char>(Storm::NetworkApplication::Storm);

	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += Storm::NetworkHelpers::toNetwork(pid);

	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += static_cast<const char>(messageType);

	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += inOutMsg;

	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += Storm::NetworkConstants::k_endOfMessageCommand;

	inOutMsg = std::move(msgWithHeader);
}

bool Storm::NetworkHelpers::parseMsg(std::string &inOutMsg, std::vector<Storm::OnMessageReceivedParam> &params)
{
	enum : std::size_t
	{
		k_minimalTokenCount = 4,
		k_lastMandatoryTokenIndex = k_minimalTokenCount - 1,
	};

	if (inOutMsg.find(Storm::NetworkConstants::k_endOfMessageCommand) != std::string::npos)
	{
		std::vector<std::string_view> splitted;
		Storm::StringAlgo::split(splitted, inOutMsg, Storm::StringAlgo::makeSplitPredicate(Storm::NetworkConstants::k_endOfMessageCommand));

		bool unprocessedLeft;
		std::size_t msgCount;
		if (inOutMsg.ends_with(Storm::NetworkConstants::k_endOfMessageCommand))
		{
			msgCount = splitted.size();
			unprocessedLeft = false;
		}
		else
		{
			msgCount = splitted.size() - 1;
			unprocessedLeft = true;
		}

		auto raiiClearer = Storm::makeLazyRAIIObject([unprocessedLeft, &splitted, &inOutMsg]()
		{
			if (unprocessedLeft)
			{
				inOutMsg = splitted.back();
			}
			else
			{
				inOutMsg.clear();
			}
		});

		params.reserve(splitted.size());

		for (std::size_t iter = 0; iter < msgCount; ++iter)
		{
			const std::string_view currentMsg = splitted[iter];

			Storm::OnMessageReceivedParam &param = params.emplace_back();

			std::vector<std::string_view> msgTokens;
			Storm::StringAlgo::split(msgTokens, currentMsg, Storm::StringAlgo::makeSplitPredicate(Storm::NetworkConstants::k_networkSeparator));

			const std::size_t msgTokenCount = msgTokens.size();
			if (msgTokenCount < k_minimalTokenCount)
			{
				Storm::throwException<Storm::Exception>("Wrong number of message tokens received! Received " + std::to_string(msgTokenCount));
			}

			const uint32_t magicKeyword = Storm::NetworkHelpers::fromNetwork<uint32_t>(msgTokens[0]);
			if (magicKeyword != Storm::NetworkConstants::k_magicKeyword)
			{
				Storm::throwException<Storm::Exception>("Wrong magic word received from connected application. Value received was " + std::to_string(magicKeyword));
			}

			param._senderId._connectedApplication = static_cast<Storm::NetworkApplication>(Storm::NetworkHelpers::fromNetwork<uint8_t>(msgTokens[1]));
			param._senderId._pid = Storm::NetworkHelpers::fromNetwork<uint32_t>(msgTokens[2]);

			param._messageType = static_cast<Storm::NetworkMessageType>(Storm::NetworkHelpers::fromNetwork<uint8_t>(msgTokens[3]));

			if (msgTokenCount > k_minimalTokenCount)
			{
				param._parameters.reserve(msgTokenCount - k_minimalTokenCount);

				for (std::size_t iter = k_minimalTokenCount; iter < msgTokenCount; ++iter)
				{
					const std::string_view msgParam = msgTokens[iter];
					if (!msgParam.empty())
					{
						param._parameters.emplace_back(msgParam);
					}
				}
			}
		}

		return true;
	}

	return false;
}

std::string Storm::NetworkHelpers::makeAuthenticationMsg()
{
	uint32_t pid = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getCurrentPID();

	constexpr std::size_t authenticationMsgSize =
		Storm::NetworkConstants::k_networkSeparator.size() * 4 +
		sizeof(Storm::NetworkConstants::k_magicKeyword) +
		sizeof(uint8_t) + // ApplicationType
		sizeof(uint32_t) + // PID
		sizeof(uint32_t) + // Version
		Storm::NetworkConstants::k_endOfMessageCommand.size() 
		;

	std::string msgWithHeader;
	msgWithHeader.reserve(authenticationMsgSize);

	msgWithHeader += Storm::NetworkHelpers::toNetwork(Storm::NetworkConstants::k_magicKeyword);

	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += static_cast<const char>(Storm::NetworkApplication::Storm);

	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += Storm::NetworkHelpers::toNetwork(pid);

	STORM_STATIC_ASSERT(Storm::NetworkConstants::k_networkVersion.size() <= sizeof(uint64_t), "The network version has more characters than what was allocated for it... This needs to be changed.");
	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += Storm::NetworkConstants::k_networkVersion;
	msgWithHeader.append(sizeof(uint64_t) - Storm::NetworkConstants::k_networkVersion.size(), '\0');

	msgWithHeader += Storm::NetworkConstants::k_networkSeparator;
	msgWithHeader += Storm::NetworkConstants::k_endOfMessageCommand;

	return msgWithHeader;
}
