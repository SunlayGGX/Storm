#pragma once


namespace Storm
{
	__forceinline bool isInsideBoundingBox(const Storm::Vector3 &minBoxVertex, const Storm::Vector3 &maxBoxVertex, const Storm::Vector3 &pos) noexcept
	{
		return
			pos.x() > minBoxVertex.x() &&
			pos.y() > minBoxVertex.y() &&
			pos.z() > minBoxVertex.z() &&
			pos.x() < maxBoxVertex.x() &&
			pos.y() < maxBoxVertex.y() &&
			pos.z() < maxBoxVertex.z()
			;
	}
}
