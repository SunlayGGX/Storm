#include "PhysicsConstraint.h"

#include "CordJoint.h"
#include "HardJoint.h"

#include "SceneConstraintConfig.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "ConstraintType.h"


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


Storm::PhysicsConstraint::PhysicsConstraint(const Storm::SceneConstraintConfig &data, physx::PxRigidActor* actor1, physx::PxRigidActor* actor2) :
	_id{ data._constraintId },
	_shouldVisualize{ data._shouldVisualize },
	_fields{ std::make_unique<Storm::UIFieldContainer>() },
	_maxDistance{ data._constraintsLength },
	_distanceFieldNameWStr{ retrievePhysicsConstraintField(_id) },
	_rb1Id{ data._rigidBodyId1 },
	_rb2Id{ data._rigidBodyId2 }
{
	switch (data._type)
	{
	case Storm::ConstraintType::HardDistanceJoint:
		_hardConstraintJoint = std::make_unique<Storm::HardJoint>(data, actor1, actor2);
		break;

	case Storm::ConstraintType::PhysicsDistanceJoint:
		_cordJointPtr = std::make_unique<Storm::CordJoint>(data, actor1, actor2);
		break;

	default:
		Storm::throwException<Storm::Exception>("Unknown constraint type! " + Storm::toStdString(data._type));
	}

	Storm::Vector3 pos1;
	Storm::Vector3 pos2;
	this->getCordPosition(pos1, pos2);

	_distanceWStr = computePhysicsConstraintDistanceWStr((pos1 - pos2).norm(), _maxDistance);

	_fields->bindFieldW(_distanceFieldNameWStr, _distanceWStr);
}

Storm::PhysicsConstraint::PhysicsConstraint(const Storm::SceneConstraintConfig &data) :
	_id{ data._constraintId },
	_maxDistance{ data._constraintsLength },
	_fields{ std::make_unique<Storm::UIFieldContainer>() },
	_distanceFieldNameWStr{ retrievePhysicsConstraintField(_id) },
	_rb1Id{ data._rigidBodyId1 },
	_rb2Id{ data._rigidBodyId2 }
{
	_fields->bindFieldW(_distanceFieldNameWStr, _distanceWStr);
}

Storm::PhysicsConstraint::~PhysicsConstraint() = default;

void Storm::PhysicsConstraint::getCordPosition(Storm::Vector3 &outPosition1, Storm::Vector3 &outPosition2) const
{
	if (_cordJointPtr)
	{
		_cordJointPtr->getJointPositionToArray(outPosition1, outPosition2);
	}
	else if (_hardConstraintJoint)
	{
		_hardConstraintJoint->getJointPositionToArray(outPosition1, outPosition2);
	}
	else
	{
		assert(false && "We shouldn't call this method with no solver!");
		outPosition1 = Storm::Vector3::Zero();
		outPosition2 = Storm::Vector3::Zero();
	}
}

void Storm::PhysicsConstraint::appendJointPositionToArray(std::vector<Storm::Vector3> &inOutJointPositions)
{
	Storm::Vector3 pos1;
	Storm::Vector3 pos2;

	this->getCordPosition(pos1, pos2);

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

Storm::Vector3 Storm::PhysicsConstraint::getForceApplied() const
{
	if (_cordJointPtr)
	{
		return _cordJointPtr->getForceApplied();
	}
	else if (_hardConstraintJoint)
	{
		return _hardConstraintJoint->getForceApplied();
	}
	else
	{
		assert(false && "We shouldn't call this method with no solver!");
		return Storm::Vector3::Zero();
	}
}

void Storm::PhysicsConstraint::setCordDistance(float distance)
{
	_distanceWStr = computePhysicsConstraintDistanceWStr(distance, _maxDistance);
	_fields->pushFieldW(_distanceFieldNameWStr);
}

void Storm::PhysicsConstraint::executeIfNeeded()
{
	if (_hardConstraintJoint)
	{
		_hardConstraintJoint->execute();
	}
}
