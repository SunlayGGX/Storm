#include "PhysicsManager.h"

#include "PhysXHandler.h"

#include "PhysicsDynamicRigidBody.h"
#include "PhysicsStaticsRigidBody.h"

#include "RigidBodySceneData.h"

#include "ThrowException.h"


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

void Storm::PhysicsManager::getMeshTransform(unsigned int meshId, Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const
{
	if (const auto staticFound = _staticsRbMap.find(meshId); staticFound != std::end(_staticsRbMap))
	{
		staticFound->second->getMeshTransform(outTrans, outRot);
	}
	else if (const auto dynamicFound = _dynamicsRbMap.find(meshId); dynamicFound != std::end(_dynamicsRbMap))
	{
		dynamicFound->second->getMeshTransform(outTrans, outRot);
	}
	else
	{
		Storm::throwException<std::exception>("Cannot find physics rb " + std::to_string(meshId));
	}
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
	if (const auto staticFound = _staticsRbMap.find(meshId); staticFound != std::end(_staticsRbMap))
	{
		staticFound->second->getMeshTransform(outTrans, outQuatRot);
	}
	else if (const auto dynamicFound = _dynamicsRbMap.find(meshId); dynamicFound != std::end(_dynamicsRbMap))
	{
		dynamicFound->second->getMeshTransform(outTrans, outQuatRot);
	}
	else
	{
		Storm::throwException<std::exception>("Cannot find physics rb " + std::to_string(meshId));
	}
}

const Storm::PhysXHandler& Storm::PhysicsManager::getPhysXHandler() const
{
	return *_physXHandler;
}

Storm::PhysXHandler& Storm::PhysicsManager::getPhysXHandler()
{
	return *_physXHandler;
}
