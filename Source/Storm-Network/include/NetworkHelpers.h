#pragma once

#include "NonInstanciable.h"
#include <boost\asio\detail\impl\socket_ops.ipp>


namespace Storm
{
	struct OnMessageReceivedParam;
	enum class NetworkMessageType;

	class NetworkHelpers : private Storm::NonInstanciable
	{
	public:
		template<class Type, class StrType>
		static Type fromNetwork(const StrType val)
		{
			if (Storm::StringAlgo::extractSize(val) == 0)
			{
				return static_cast<Type>(0);
			}

			const auto rawText = Storm::StringAlgo::extractRaw(val);
			if constexpr (sizeof(Type) == 1)
			{
				return static_cast<Type>(*rawText);
			}
			else if constexpr (sizeof(Type) == 2)
			{
				return static_cast<Type>(boost::asio::detail::socket_ops::network_to_host_short(*reinterpret_cast<const uint16_t*>(rawText)));
			}
			else if constexpr (sizeof(Type) == 4)
			{
				return static_cast<Type>(boost::asio::detail::socket_ops::network_to_host_long(*reinterpret_cast<const uint32_t*>(rawText)));
			}
			else if constexpr (sizeof(Type) == 8)
			{
				return
					static_cast<Type>(boost::asio::detail::socket_ops::network_to_host_long(*reinterpret_cast<const uint32_t*>(rawText))) << 4 |
					static_cast<Type>(boost::asio::detail::socket_ops::network_to_host_long(*reinterpret_cast<const uint32_t*>(rawText + 4)));
			}
			else
			{
				STORM_COMPILE_ERROR("Cannot convert Type from network to host!");
			}
		}

		template<class Type>
		static std::string toNetwork(const Type val)
		{
			if constexpr (sizeof(Type) == 1)
			{
				return std::string{ val, 1 };
			}
			else if constexpr (sizeof(Type) == 2)
			{
				uint16_t tmp = boost::asio::detail::socket_ops::host_to_network_short(val);
				return std::string{ reinterpret_cast<const char*>(&tmp), 2 };
			}
			else if constexpr (sizeof(Type) == 4)
			{
				uint32_t tmp = boost::asio::detail::socket_ops::host_to_network_long(val);
				return std::string{ reinterpret_cast<const char*>(&tmp), 4 };
			}
			else if constexpr (sizeof(Type) == 8)
			{
				uint8_t bytes[8];
				uint32_t* current = reinterpret_cast<uint32_t*>(bytes);
				*current = boost::asio::detail::socket_ops::host_to_network_long(static_cast<uint32_t>(val >> 4));
				*(current + 1) = boost::asio::detail::socket_ops::host_to_network_long(static_cast<uint32_t>(val));

				return std::string{ reinterpret_cast<const char*>(bytes), 8 };
			}
			else
			{
				STORM_COMPILE_ERROR("Cannot convert Type from network to host!");
			}
		}

	public:
		static void prepareMsg(std::string &inOutMsg, const Storm::NetworkMessageType messageType);
		static bool parseMsg(std::string &inOutMsg, std::vector<Storm::OnMessageReceivedParam> &params);

		static std::string makeAuthenticationMsg();
	};
}
