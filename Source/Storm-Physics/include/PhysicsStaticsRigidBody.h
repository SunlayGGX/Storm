#pragma once

#include "RigidBodyHolder.h"
#include "PhysicalShape.h"

#include "UniquePointer.h"


namespace Storm
{
	struct RigidBodySceneData;

	class PhysicsStaticsRigidBody :
		public Storm::RigidBodyHolder,
		public Storm::PhysicalShape
	{
	public:
		PhysicsStaticsRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices);

	private:
		Storm::UniquePointer<physx::PxRigidStatic> _internalRb;
	};
}
