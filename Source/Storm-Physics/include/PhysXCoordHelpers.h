#pragma once


namespace Storm
{
	_forceinline physx::PxVec3 convertToPx(const Storm::Vector3 &vec)
	{
		return physx::PxVec3{ vec.x(), vec.y(), vec.z() };
	}

	_forceinline Storm::Vector3 convertToStorm(const physx::PxVec3 &vec)
	{
		return Storm::Vector3{ vec.x, vec.y, vec.z };
	}
}
