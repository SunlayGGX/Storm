#pragma once

#include "RigidBodyHolder.h"

#include "UniquePointer.h"


namespace Storm
{
	struct RigidBodySceneData;

	class PhysicsStaticsRigidBody : public Storm::RigidBodyHolder
	{
	public:
		PhysicsStaticsRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices);

	private:
		Storm::UniquePointer<physx::PxRigidStatic> _internalRb;
	};
}
