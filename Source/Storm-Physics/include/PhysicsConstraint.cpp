#include "PhysicsConstraint.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"

#include "ConstraintData.h"

#include "PhysXCoordHelpers.h"


namespace
{
	Storm::UniquePointer<physx::PxJoint> makeDistanceJoint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2)
	{
		Storm::PhysXHandler &physxHandler = Storm::PhysicsManager::instance().getPhysXHandler();
		return physxHandler.createDistanceJoint(data, actor1, actor2);
	}
}


Storm::PhysicsConstraint::PhysicsConstraint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2) :
	_id{ data._constraintId },
	_internalJointPtr{ makeDistanceJoint(data, actor1, actor2) },
	_shouldVisualize{ data._shouldVisualize }
{

}

Storm::PhysicsConstraint::~PhysicsConstraint() = default;

void Storm::PhysicsConstraint::appendJointPositionToArray(std::vector<Storm::Vector3> &inOutJointPositions) const
{
	if (_shouldVisualize)
	{
		physx::PxRigidActor* actor0;
		physx::PxRigidActor* actor1;
		_internalJointPtr->getActors(actor0, actor1);

		physx::PxTransform actor0GlobalPos = actor0->getGlobalPose();
		physx::PxTransform actor1GlobalPos = actor1->getGlobalPose();

		physx::PxTransform jointTransformActor0 = _internalJointPtr->getLocalPose(physx::PxJointActorIndex::Enum::eACTOR0);
		physx::PxTransform jointTransformActor1 = _internalJointPtr->getLocalPose(physx::PxJointActorIndex::Enum::eACTOR1);

		inOutJointPositions.emplace_back(Storm::convertToStorm(actor0GlobalPos.p + jointTransformActor0.p));
		inOutJointPositions.emplace_back(Storm::convertToStorm(actor1GlobalPos.p + jointTransformActor1.p));
	}
}

std::size_t Storm::PhysicsConstraint::getID() const noexcept
{
	return _id;
}
