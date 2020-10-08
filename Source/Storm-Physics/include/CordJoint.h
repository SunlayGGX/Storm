#pragma once

#include "JointBase.h"
#include "UniquePointer.h"


namespace Storm
{
	struct ConstraintData;

	class CordJoint : public Storm::JointBase
	{
	public:
		CordJoint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);

	public:
		void getJointPositionToArray(Storm::Vector3 &outPos1, Storm::Vector3 &outPos2) const;

	private:
		Storm::UniquePointer<physx::PxJoint> _internalDistanceJointPtr;
		Storm::UniquePointer<physx::PxJoint> _internalRevoluteJoint0Ptr;
		Storm::UniquePointer<physx::PxJoint> _internalRevoluteJoint1Ptr;
	};
}
