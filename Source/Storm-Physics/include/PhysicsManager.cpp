#include "PhysicsManager.h"

#include "PhysXHandler.h"

#include "PhysicsDynamicRigidBody.h"
#include "PhysicsStaticsRigidBody.h"

#include "PhysicsConstraint.h"

#include "RigidBodySceneData.h"
#include "ConstraintData.h"

#include "ThrowException.h"
#include "SearchAlgo.h"


Storm::PhysicsManager::PhysicsManager() = default;
Storm::PhysicsManager::~PhysicsManager() = default;

void Storm::PhysicsManager::initialize_Implementation()
{
	LOG_COMMENT << "PhysX initialization started";

	_physXHandler = std::make_unique<Storm::PhysXHandler>();

	LOG_COMMENT << "PhysX initialization ended";
}

void Storm::PhysicsManager::cleanUp_Implementation()
{
	LOG_COMMENT << "PhysX cleanup requested";

	_constraints.clear();

	_staticsRbMap.clear();
	_dynamicsRbMap.clear();

	_physXHandler.reset();

	LOG_COMMENT << "PhysX cleanup finished successfully";
}

void Storm::PhysicsManager::update(float deltaTime)
{
	_physXHandler->update(_simulationMutex, deltaTime);
}

void Storm::PhysicsManager::addPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertexes)
{
	if (rbSceneData._static)
	{
		_staticsRbMap[rbSceneData._rigidBodyID] = std::make_unique<Storm::PhysicsStaticsRigidBody>(rbSceneData, vertexes);
	}
	else
	{
		_dynamicsRbMap[rbSceneData._rigidBodyID] = std::make_unique<Storm::PhysicsDynamicRigidBody>(rbSceneData, vertexes);
	}
}

void Storm::PhysicsManager::bindParentRbToPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::shared_ptr<Storm::IRigidBody> &parentRb) const
{
	if (rbSceneData._static)
	{
		if (const auto found = _staticsRbMap.find(rbSceneData._rigidBodyID); found != std::end(_staticsRbMap))
		{
			found->second->setRbParent(parentRb);
		}
		else
		{
			Storm::throwException<std::exception>("Cannot find static physics rb " + std::to_string(rbSceneData._rigidBodyID));
		}
	}
	else
	{
		if (const auto found = _dynamicsRbMap.find(rbSceneData._rigidBodyID); found != std::end(_dynamicsRbMap))
		{
			found->second->setRbParent(parentRb);
		}
		else
		{
			Storm::throwException<std::exception>("Cannot find dynamic physics rb " + std::to_string(rbSceneData._rigidBodyID));
		}
	}
}

void Storm::PhysicsManager::addConstraint(const Storm::ConstraintData &constraintData)
{
	STORM_NOT_IMPLEMENTED;

	Storm::SearchAlgo::executeOnObjectInContainer(constraintData._rigidBodyId1, [&](const auto &rb1)
	{
		Storm::SearchAlgo::executeOnObjectInContainer(constraintData._rigidBodyId2, [&](const auto &rb2)
		{


		}, _staticsRbMap, _dynamicsRbMap);
	}, _staticsRbMap, _dynamicsRbMap);

	LOG_DEBUG << "Constraint loaded";
}

void Storm::PhysicsManager::loadConstraints(const std::vector<Storm::ConstraintData> &constraintsToLoad)
{
	const std::size_t constraintsCount = constraintsToLoad.size();
	if (constraintsCount > 0)
	{
		LOG_DEBUG << "Loading " << constraintsCount << " constraints";

		_constraints.reserve(_constraints.size() + constraintsCount);

		for (const Storm::ConstraintData &constraint : constraintsToLoad)
		{
			this->addConstraint(constraint);
		}
	}
	else
	{
		LOG_DEBUG << "No constraints to be load";
	}
}

void Storm::PhysicsManager::getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const
{
	Storm::SearchAlgo::executeOnObjectInContainer(meshId, [&outTrans, &outRot](const auto &rbFound)
	{
		rbFound.getMeshTransform(outTrans, outRot);
	}, _staticsRbMap, _dynamicsRbMap);
}

void Storm::PhysicsManager::applyLocalForces(unsigned int particleSystemId, const std::vector<Storm::Vector3> &position, const std::vector<Storm::Vector3> &force)
{
	if (const auto dynamicFound = _dynamicsRbMap.find(particleSystemId); dynamicFound != std::end(_dynamicsRbMap))
	{
		Storm::PhysicsDynamicRigidBody &dynamicRb = *dynamicFound->second;
		const std::size_t applyCount = position.size();

		assert(applyCount == force.size() && "Mismatch detected between position and force apply count.");
		for (std::size_t iter = 0; iter < applyCount; ++iter)
		{
			dynamicRb.applyForce(position[iter], force[iter]);
		}
	}
	else
	{
		assert(_staticsRbMap.find(particleSystemId) != std::end(_staticsRbMap) && "Cannot find requested physics rigid body!");
	}
}

void Storm::PhysicsManager::getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const
{
	Storm::SearchAlgo::executeOnObjectInContainer(meshId, [&outTrans, &outQuatRot](const auto &rbFound)
	{
		rbFound.getMeshTransform(outTrans, outQuatRot);
	}, _staticsRbMap, _dynamicsRbMap);
}

const Storm::PhysXHandler& Storm::PhysicsManager::getPhysXHandler() const
{
	return *_physXHandler;
}

Storm::PhysXHandler& Storm::PhysicsManager::getPhysXHandler()
{
	return *_physXHandler;
}
