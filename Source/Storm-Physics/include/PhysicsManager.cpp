#include "PhysicsManager.h"

#include "PhysXHandler.h"
#include "PhysicsDynamicRigidBody.h"

#include "RigidBodySceneData.h"


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
	
	_physXHandler.reset();

	LOG_COMMENT << "PhysX cleanup finished successfully";
}

void Storm::PhysicsManager::addPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertexes)
{
	if (rbSceneData._static)
	{
		// TODO
	}
	else
	{
		_dynamicsRbMap[rbSceneData._rigidBodyID] = std::make_unique<Storm::PhysicsDynamicRigidBody>(rbSceneData, vertexes);
	}
}

void Storm::PhysicsManager::bindParentRbToPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::shared_ptr<Storm::IRigidBody> &parentRb)
{
	if (rbSceneData._static)
	{
		// TODO
	}
	else
	{
		_dynamicsRbMap[rbSceneData._rigidBodyID]->setRbParent(parentRb);
	}
}
