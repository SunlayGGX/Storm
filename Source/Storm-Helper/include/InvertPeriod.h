#pragma once


namespace Storm
{
	template<class RatioPeriod>
	struct InvertPeriod;

	template<class Type, Type num, Type denum>
	struct InvertPeriod<std::ratio<num, denum>>
	{
		static constexpr float value = static_cast<float>(denum) / static_cast<float>(num);
	};
}
