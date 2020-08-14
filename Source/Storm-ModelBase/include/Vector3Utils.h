#pragma once

#include "StaticAssertionsMacros.h"


namespace Storm
{
	template<class SelectorFunc>
	void minInPlace(Storm::Vector3 &toBeMin, const Storm::Vector3 &other, const SelectorFunc &selector)
	{
		STORM_STATIC_ASSERT(
			std::is_reference_v<decltype(selector(toBeMin))>,
			"Selector should return a reference or we would modify a value instead (will be lost when the function exit)."
		);

		auto &val = selector(toBeMin);
		const auto &toCheck = selector(other);
		if (val > toCheck)
		{
			val = toCheck;
		}
	}

	template<class SelectorFunc>
	void maxInPlace(Storm::Vector3 &toBeMax, const Storm::Vector3 &other, const SelectorFunc &selector)
	{
		STORM_STATIC_ASSERT(
			std::is_reference_v<decltype(selector(toBeMax))>,
			"Selector should return a reference or we would modify a value instead (will be lost when the function exit)."
		);

		auto &val = selector(toBeMax);
		const auto &toCheck = selector(other);
		if (val < toCheck)
		{
			val = toCheck;
		}
	}

	template<class SelectorFunc>
	void minMaxInPlace(Storm::Vector3 &toBeMin, Storm::Vector3 &toBeMax, const Storm::Vector3 &other, const SelectorFunc &selector)
	{
		minInPlace(toBeMin, other, selector);
		maxInPlace(toBeMax, other, selector);
	}
}
