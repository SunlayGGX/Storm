#include "PhysicsConstraint.h"

#include "CordJoint.h"

#include "ConstraintData.h"


Storm::PhysicsConstraint::PhysicsConstraint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2) :
	_id{ data._constraintId },
	_cordJointPtr{ std::make_unique<Storm::CordJoint>(data, actor1, actor2) },
	_shouldVisualize{ data._shouldVisualize }
{

}

Storm::PhysicsConstraint::~PhysicsConstraint() = default;

void Storm::PhysicsConstraint::appendJointPositionToArray(std::vector<Storm::Vector3> &inOutJointPositions) const
{
	Storm::Vector3 pos1;
	Storm::Vector3 pos2;

	_cordJointPtr->getJointPositionToArray(pos1, pos2);

	if (_shouldVisualize)
	{
		inOutJointPositions.emplace_back(pos1);
		inOutJointPositions.emplace_back(pos2);
	}
}

std::size_t Storm::PhysicsConstraint::getID() const noexcept
{
	return _id;
}
