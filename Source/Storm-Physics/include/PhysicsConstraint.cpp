#include "PhysicsConstraint.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"


namespace
{
	Storm::UniquePointer<physx::PxJoint> makeJoint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2)
	{
		Storm::PhysXHandler &physxHandler = Storm::PhysicsManager::instance().getPhysXHandler();
		return physxHandler.createJoint(data, actor1, actor2);
	}
}


Storm::PhysicsConstraint::PhysicsConstraint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2) :
	_internalJointPtr{ makeJoint(data, actor1, actor2) }
{

}

Storm::PhysicsConstraint::~PhysicsConstraint() = default;
