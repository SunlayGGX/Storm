#pragma once

#include "UniquePointer.h"


namespace Storm
{
	struct SceneConstraintConfig;

	class PhysXJointBase
	{
	public:
		virtual ~PhysXJointBase() = default;

	public:
		static Storm::UniquePointer<physx::PxJoint> makeDistanceJoint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);
		static std::pair<Storm::UniquePointer<physx::PxJoint>, Storm::UniquePointer<physx::PxJoint>> makeSpinnableJoint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);
	};
}
