#pragma once

#include "PhysXJointBase.h"
#include "UniquePointer.h"


namespace Storm
{
	struct SceneConstraintConfig;

	class CordJoint : public Storm::PhysXJointBase
	{
	public:
		CordJoint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);

	public:
		void getJointPositionToArray(Storm::Vector3 &outPos1, Storm::Vector3 &outPos2) const;
		Storm::Vector3 getForceApplied() const;

	private:
		Storm::UniquePointer<physx::PxJoint> _internalDistanceJointPtr;
		Storm::UniquePointer<physx::PxJoint> _internalRevoluteJoint0Ptr;
		Storm::UniquePointer<physx::PxJoint> _internalRevoluteJoint1Ptr;
	};
}
