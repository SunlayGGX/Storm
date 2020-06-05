#include "PhysicsManager.h"

#include "PhysXHandler.h"


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
	throw std::logic_error("The method or operation is not implemented.");
}

void Storm::PhysicsManager::bindParentRbToPhysicalBody(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb)
{
	throw std::logic_error("The method or operation is not implemented.");
}
