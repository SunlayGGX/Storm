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

	// Init the Vector3 to use into max algorithm (get the max value, so it is important that the value is the lowest possible value).
	inline Storm::Vector3 initVector3ForMax()
	{
		return Storm::Vector3{ std::numeric_limits<Storm::Vector3::Scalar>::lowest(), std::numeric_limits<Storm::Vector3::Scalar>::lowest(), std::numeric_limits<Storm::Vector3::Scalar>::lowest() };
	}

	// Init the Vector3 to use into min algorithm (get the max value, so it is important that the value is the maximum possible value).
	inline Storm::Vector3 initVector3ForMin()
	{
		return Storm::Vector3{ std::numeric_limits<Storm::Vector3::Scalar>::max(), std::numeric_limits<Storm::Vector3::Scalar>::max(), std::numeric_limits<Storm::Vector3::Scalar>::max() };
	}

	// Set toBeMinNeg selected member to the farthest from 0 in negative. If both vect1 and vec2 are negative, then 0 is set. 
	template<class SelectorFunc>
	void minNegativeInPlaceFromBoth(Storm::Vector3 &toBeMinNeg, const Storm::Vector3 &vec1, const Storm::Vector3 &vec2, const SelectorFunc &selector)
	{
		selector(toBeMinNeg) = std::min(std::min(selector(vec1), selector(vec2)), 0.f);
	}
}
