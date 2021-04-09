#include "HardJoint.h"

#include "SceneConstraintConfig.h"

#include "PhysXCoordHelpers.h"


namespace
{
	__forceinline void storeLinkHookPosition(const physx::PxTransform &actorTransf, const Storm::Vector3 &localOffset, Storm::Vector3 &outPos)
	{
		outPos = Storm::convertToStorm(actorTransf.p);
		outPos += localOffset;
	}

	__forceinline void storeLinkHookPosition(const physx::PxRigidActor &actor, const Storm::Vector3 &localOffset, Storm::Vector3 &outPos)
	{
		storeLinkHookPosition(actor.getGlobalPose(), localOffset, outPos);
	}
}


Storm::HardJoint::HardJoint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2) :
	_actor1{ actor1 },
	_actor2{ actor2 },
	_actor1LinkHookOffset{ data._rigidBody1LinkTranslationOffset },
	_actor2LinkHookOffset{ data._rigidBody2LinkTranslationOffset },
	_hardDistSquared{ data._constraintsLength * data._constraintsLength },
	_hardCoeff{ -data._constraintsLength }
{

}

void Storm::HardJoint::getJointPositionToArray(Storm::Vector3 &outPos1, Storm::Vector3 &outPos2) const
{
	storeLinkHookPosition(*_actor1, _actor1LinkHookOffset, outPos1);
	storeLinkHookPosition(*_actor2, _actor2LinkHookOffset, outPos2);
}

Storm::Vector3 Storm::HardJoint::getForceApplied() const
{
	// We solve the constraint ourself by blocking the position instead of resolving with penalty forces like PhysX does.
	// It allows the joint to be really non elastic and springy. The downside is that we cannot compute a force as-is...
	return Storm::Vector3::Zero();
}

void Storm::HardJoint::execute()
{
	physx::PxTransform actor1Transform = _actor1->getGlobalPose();
	physx::PxTransform actor2Transform = _actor2->getGlobalPose();

	Storm::Vector3 link1HookPos;
	storeLinkHookPosition(actor1Transform, _actor1LinkHookOffset, link1HookPos);

	Storm::Vector3 link2HookPos;
	storeLinkHookPosition(actor2Transform, _actor2LinkHookOffset, link2HookPos);

	Storm::Vector3 link = link1HookPos - link2HookPos;
	const float distanceSquared = link.squaredNorm();

	if (distanceSquared > _hardDistSquared)
	{
		const float coeff = _hardCoeff / std::sqrtf(distanceSquared);
		link *= coeff;
		
		// This suppose actor1 is fixed. If it isn't, then we should consider it (but since it is never the case for what I need, I'll take some shortcut (YAGNI)).
		actor2Transform.p = actor1Transform.p + Storm::convertToPx(_actor1LinkHookOffset + link - _actor2LinkHookOffset);

		_actor2->setGlobalPose(actor2Transform);
	}
}

