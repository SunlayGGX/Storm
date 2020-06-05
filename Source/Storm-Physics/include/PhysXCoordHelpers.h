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

	_forceinline physx::PxQuat convertToPxRotation(const Storm::Vector3 &rot)
	{
		physx::PxQuat result;

		constexpr const float midDegreeToRadianCoeff = static_cast<float>(M_PI / 180.0 * 0.5) ;

		const float midRoll = rot.z() * midDegreeToRadianCoeff;
		const float midPitch = rot.x() * midDegreeToRadianCoeff;
		const float midYaw = rot.y() * midDegreeToRadianCoeff;

		// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
		const float cy = std::cosf(midYaw);
		const float sy = std::sinf(midYaw);
		const float cp = std::cosf(midPitch);
		const float sp = std::sinf(midPitch);
		const float cr = std::cosf(midRoll);
		const float sr = std::sinf(midRoll);

		result.w = cr * cp * cy + sr * sp * sy;
		result.x = sr * cp * cy - cr * sp * sy;
		result.y = cr * sp * cy + sr * cp * sy;
		result.z = cr * cp * sy - sr * sp * cy;

		return result;
	}

	_forceinline physx::PxTransform convertToPx(const Storm::Vector3 &translation, const Storm::Vector3 &eulerRot)
	{
		physx::PxTransform result;

		result.p = Storm::convertToPx(translation);
		result.q = Storm::convertToPxRotation(eulerRot);

		return result;
	}
}
