#include "JointBase.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"


Storm::UniquePointer<physx::PxJoint> Storm::JointBase::makeDistanceJoint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2)
{
	Storm::PhysXHandler &physxHandler = Storm::PhysicsManager::instance().getPhysXHandler();
	return physxHandler.createDistanceJoint(data, actor1, actor2);
}
