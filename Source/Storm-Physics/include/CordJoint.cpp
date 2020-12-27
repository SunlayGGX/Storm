#include "CordJoint.h"

#include "SceneConstraintConfig.h"

#include "PhysXCoordHelpers.h"


Storm::CordJoint::CordJoint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2) :
	_internalDistanceJointPtr{ Storm::JointBase::makeDistanceJoint(data, actor1, actor2) }
{
	if (data._preventRotations)
	{
		auto spinnableJoints = Storm::JointBase::makeSpinnableJoint(data, actor1, actor2);
		_internalRevoluteJoint0Ptr = std::move(spinnableJoints.first);
		_internalRevoluteJoint1Ptr = std::move(spinnableJoints.second);
	}
}

void Storm::CordJoint::getJointPositionToArray(Storm::Vector3 &outPos1, Storm::Vector3 &outPos2) const
{
	physx::PxRigidActor* actor0;
	physx::PxRigidActor* actor1;
	_internalDistanceJointPtr->getActors(actor0, actor1);

	physx::PxTransform actor0GlobalPos = actor0->getGlobalPose();
	physx::PxTransform actor1GlobalPos = actor1->getGlobalPose();

	physx::PxTransform jointTransformActor0 = _internalDistanceJointPtr->getLocalPose(physx::PxJointActorIndex::Enum::eACTOR0);
	physx::PxTransform jointTransformActor1 = _internalDistanceJointPtr->getLocalPose(physx::PxJointActorIndex::Enum::eACTOR1);

	outPos1 = Storm::convertToStorm(actor0GlobalPos.p + jointTransformActor0.p);
	outPos2 = Storm::convertToStorm(actor1GlobalPos.p + jointTransformActor1.p);
}

Storm::Vector3 Storm::CordJoint::getForceApplied() const
{
	physx::PxVec3 force;
	physx::PxVec3 torque;

	_internalDistanceJointPtr->getConstraint()->getForce(force, torque);

	return Storm::convertToStorm(force);
}
