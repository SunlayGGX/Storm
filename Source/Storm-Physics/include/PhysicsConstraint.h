#pragma once

#include "UniquePointer.h"


namespace Storm
{
	struct ConstraintData;
	struct ConstraintVisualizationItem;

	// Link constraint... Some other constraint type exist but won't I use them (I don't need them, YAGNI), so PhysicsConstraint == Distance joint.
	class PhysicsConstraint
	{
	public:
		PhysicsConstraint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2);
		~PhysicsConstraint();

	public:
		void appendJointPositionToArray(std::vector<Storm::ConstraintVisualizationItem> &inOutJointPosition) const;
		std::size_t getID() const noexcept;

	private:
		const std::size_t _id;
		Storm::UniquePointer<physx::PxJoint> _internalJointPtr;
		bool _shouldVisualize;
	};
}
