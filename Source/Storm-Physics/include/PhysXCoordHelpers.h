#pragma once


namespace Storm
{
	__forceinline physx::PxVec3 convertToPx(const Storm::Vector3 &vec)
	{
		return physx::PxVec3{ vec.x(), vec.y(), vec.z() };
	}

	__forceinline Storm::Vector3 convertToStorm(const physx::PxVec3 &vec)
	{
		return Storm::Vector3{ vec.x, vec.y, vec.z };
	}

	template<class Ret>
	__forceinline Ret convertToStorm(const physx::PxQuat &q);

	__forceinline physx::PxQuat convertToPxRotation(const Storm::Vector3 &rot)
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

	template<>
	__forceinline Storm::Vector3 convertToStorm<Storm::Vector3>(const physx::PxQuat &q)
	{
		Storm::Vector3 result;

		constexpr const float radToDegreeCoeff = static_cast<float>(180.0 / M_PI);

		// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

		// roll (x-axis rotation)
		float sinrCosp = 2.f * (q.w * q.x + q.y * q.z);
		float cosrCosp = 1.f - 2.f * (q.x * q.x + q.y * q.y);
		result.x() = radToDegreeCoeff * std::atan2(sinrCosp, cosrCosp);

		// pitch (y-axis rotation)
		float sinp = 2.f * (q.w * q.y - q.z * q.x);
		if (std::abs(sinp) >= 1.f)
		{
			result.y() = radToDegreeCoeff * std::copysign(static_cast<float>(M_PI_2), sinp); // use 90 degrees if out of range
		}
		else
		{
			result.y() = radToDegreeCoeff * std::asin(sinp);
		}

		// yaw (z-axis rotation)
		float sinyCosp = 2.f * (q.w * q.z + q.x * q.y);
		float cosyCosp = 1.f - 2.f * (q.y * q.y + q.z * q.z);
		result.z() = radToDegreeCoeff * std::atan2(sinyCosp, cosyCosp);

		return result;
	}

	template<>
	__forceinline Storm::Quaternion convertToStorm<Storm::Quaternion>(const physx::PxQuat &q)
	{
		return Storm::Quaternion{ q.w, q.x, q.y, q.z };
	}

	__forceinline physx::PxTransform convertToPx(const Storm::Vector3 &translation, const Storm::Vector3 &eulerRot)
	{
		physx::PxTransform result;

		result.p = Storm::convertToPx(translation);
		result.q = Storm::convertToPxRotation(eulerRot);

		return result;
	}
}
