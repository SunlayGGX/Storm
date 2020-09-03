#pragma once

#include "UniquePointer.h"


namespace Storm
{
	struct ConstraintData;

	class PhysicsConstraint
	{
	public:
		PhysicsConstraint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);
		~PhysicsConstraint();

	private:
		Storm::UniquePointer<physx::PxJoint> _internalJointPtr;
	};
}
