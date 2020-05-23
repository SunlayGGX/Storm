#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	template<class RatioPeriod>
	struct InvertPeriod;

	template<class Type, Type num, Type denum>
	struct InvertPeriod<std::ratio<num, denum>>
	{
		static constexpr float value = static_cast<float>(denum) / static_cast<float>(num);
	};

	struct ChronoHelper : private Storm::NonInstanciable
	{
	public:
		template<std::size_t bufferAverizeSize = 1, class DurationType>
		static float toFps(const DurationType &duration)
		{
			constexpr float coefficientDurationToFps = Storm::InvertPeriod<DurationType::period>::value * static_cast<float>(bufferAverizeSize);
			return coefficientDurationToFps / static_cast<float>(duration.count());
		}
	};
}
