#include "PhysicsConstraint.h"

#include "CordJoint.h"

#include "ConstraintData.h"

#include "UIField.h"
#include "UIFieldContainer.h"


namespace
{
	std::wstring retrievePhysicsConstraintField(const std::size_t constraintID)
	{
		return L"Constraint " + std::to_wstring(constraintID);
	}

	std::wstring computePhysicsConstraintDistanceWStr(float distance, const float maxDistance)
	{
		std::wstring result;
		
		const std::wstring distWStr{ std::to_wstring(distance) };
		const std::wstring distPercentWStr{ std::to_wstring(distance / maxDistance * 100.f) };
		result.reserve(5 + distWStr.size() + distPercentWStr.size());

		result += distWStr;
		result += L"m (";
		result += distPercentWStr;
		result += L"%)";

		return result;
	}
}


Storm::PhysicsConstraint::PhysicsConstraint(const Storm::ConstraintData &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2) :
	_id{ data._constraintId },
	_cordJointPtr{ std::make_unique<Storm::CordJoint>(data, actor1, actor2) },
	_shouldVisualize{ data._shouldVisualize },
	_fields{ std::make_unique<Storm::UIFieldContainer>() },
	_maxDistance{ data._constraintsLength },
	_distanceFieldNameWStr{ retrievePhysicsConstraintField(_id) }
{
	Storm::Vector3 pos1;
	Storm::Vector3 pos2;
	_cordJointPtr->getJointPositionToArray(pos1, pos2);

	_distanceWStr = computePhysicsConstraintDistanceWStr((pos1 - pos2).norm(), _maxDistance);

	_fields->bindFieldW(_distanceFieldNameWStr, _distanceWStr);
}

Storm::PhysicsConstraint::~PhysicsConstraint() = default;

void Storm::PhysicsConstraint::appendJointPositionToArray(std::vector<Storm::Vector3> &inOutJointPositions)
{
	Storm::Vector3 pos1;
	Storm::Vector3 pos2;

	_cordJointPtr->getJointPositionToArray(pos1, pos2);

	if (_shouldVisualize)
	{
		inOutJointPositions.emplace_back(pos1);
		inOutJointPositions.emplace_back(pos2);
	}

	this->setCordDistance((pos1 - pos2).norm());
}

std::size_t Storm::PhysicsConstraint::getID() const noexcept
{
	return _id;
}

void Storm::PhysicsConstraint::setCordDistance(float distance)
{
	_distanceWStr = computePhysicsConstraintDistanceWStr(distance, _maxDistance);
	_fields->pushFieldW(_distanceFieldNameWStr);
}
