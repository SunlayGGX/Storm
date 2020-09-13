#include "CordJoint.h"

#include "ConstraintData.h"

#include "PhysXCoordHelpers.h"


Storm::CordJoint::CordJoint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2) :
	_internalDistanceJointPtr{ Storm::JointBase::makeDistanceJoint(data, actor1, actor2) }
{
	if (data._preventRotations)
	{
		auto spinnableJoints = Storm::JointBase::makeSpinnableJoint(data, actor1, actor2);
		_internalRevoluteJoint0Ptr = std::move(spinnableJoints.first);
		_internalRevoluteJoint1Ptr = std::move(spinnableJoints.second);
	}
}

void Storm::CordJoint::appendJointPositionToArray(std::vector<Storm::Vector3> &inOutJointPositions) const
{
	physx::PxRigidActor* actor0;
	physx::PxRigidActor* actor1;
	_internalDistanceJointPtr->getActors(actor0, actor1);

	physx::PxTransform actor0GlobalPos = actor0->getGlobalPose();
	physx::PxTransform actor1GlobalPos = actor1->getGlobalPose();

	physx::PxTransform jointTransformActor0 = _internalDistanceJointPtr->getLocalPose(physx::PxJointActorIndex::Enum::eACTOR0);
	physx::PxTransform jointTransformActor1 = _internalDistanceJointPtr->getLocalPose(physx::PxJointActorIndex::Enum::eACTOR1);

	inOutJointPositions.emplace_back(Storm::convertToStorm(actor0GlobalPos.p + jointTransformActor0.p));
	inOutJointPositions.emplace_back(Storm::convertToStorm(actor1GlobalPos.p + jointTransformActor1.p));
}
