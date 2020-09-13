#pragma once

#include "UniquePointer.h"


namespace Storm
{
	struct ConstraintData;

	class JointBase
	{
	public:
		virtual ~JointBase() = default;

	public:
		static Storm::UniquePointer<physx::PxJoint> makeDistanceJoint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);
		static std::pair<Storm::UniquePointer<physx::PxJoint>, Storm::UniquePointer<physx::PxJoint>>  makeSpinnableJoint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);
	};
}
