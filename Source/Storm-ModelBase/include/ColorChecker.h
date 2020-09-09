#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class ColorCheckerHelper : private Storm::NonInstanciable
	{
	public:
		template<class ChannelValueType>
		static __forceinline bool channelIsInvalid(const ChannelValueType val) noexcept
		{
			return val < static_cast<ChannelValueType>(0) || val > static_cast<ChannelValueType>(1);
		}

		template<class ChannelValueType>
		static __forceinline bool channelIsValid(const ChannelValueType val) noexcept
		{
			return !Storm::ColorCheckerHelper::channelIsInvalid(val);
		}

	public:
		static bool isInvalid(const Storm::Vector3 &color3) noexcept;
		static bool isInvalid(const Storm::Vector4 &color4) noexcept;

	public:
		template<class ColorType>
		static __forceinline bool isValid(const ColorType &color) noexcept
		{
			return !Storm::ColorCheckerHelper::channelIsInvalid(color);
		}
	};
}
