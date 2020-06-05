#include "PhysicsStaticsRigidBody.h"

#include "ThrowException.h"
#include "PhysXCoordHelpers.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"

#include "RigidBodySceneData.h"

#include <PxRigidStatic.h>


namespace
{
	Storm::UniquePointer<physx::PxRigidStatic> createStaticRigidBody(const Storm::RigidBodySceneData &rbSceneData)
	{
		auto &physicsHandler = Storm::PhysicsManager::instance().getPhysXHandler();
		return physicsHandler.createStaticRigidBody(rbSceneData);
	}
}


Storm::PhysicsStaticsRigidBody::PhysicsStaticsRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices) :
	_internalRb{ createStaticRigidBody(rbSceneData) }
{
	if (!_internalRb)
	{
		Storm::throwException<std::exception>("PhysX failed to create the internal rigid body for object " + std::to_string(rbSceneData._rigidBodyID));
	}
}

