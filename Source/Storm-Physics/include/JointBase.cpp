#include "JointBase.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"


Storm::UniquePointer<physx::PxJoint> Storm::JointBase::makeDistanceJoint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2)
{
	Storm::PhysXHandler &physxHandler = Storm::PhysicsManager::instance().getPhysXHandler();
	return physxHandler.createDistanceJoint(data, actor1, actor2);
}

std::pair<Storm::UniquePointer<physx::PxJoint>, Storm::UniquePointer<physx::PxJoint>> Storm::JointBase::makeSpinnableJoint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2)
{
	Storm::PhysXHandler &physxHandler = Storm::PhysicsManager::instance().getPhysXHandler();
	return physxHandler.createSpinnableJoint(data, actor1, actor2);
}
