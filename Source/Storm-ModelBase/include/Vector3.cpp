#include "Vector3.h"

namespace
{
	__forceinline float reduceTo_0_2pi_interval(float angle)
	{
		constexpr float twoPi = static_cast<float>(2.0 * M_PI);
		return fmodf(angle, twoPi);
	}
}

extern bool operator==(const Storm::Rotation &left, const Storm::Rotation &right)
{
	return
		left.axis() == right.axis() &&
		reduceTo_0_2pi_interval(left.angle()) == reduceTo_0_2pi_interval(right.angle());
}

extern bool operator!=(const Storm::Rotation &left, const Storm::Rotation &right)
{
	return !(left == right);
}
