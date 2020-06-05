#pragma once

#include "RigidBodyHolder.h"

#include "UniquePointer.h"


namespace Storm
{
	struct RigidBodySceneData;

	class PhysicsDynamicRigidBody : public Storm::RigidBodyHolder
	{
	public:
		PhysicsDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices);

	private:
		Storm::UniquePointer<physx::PxRigidDynamic> _internalRb;
	};
}
