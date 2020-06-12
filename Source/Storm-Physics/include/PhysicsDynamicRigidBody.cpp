#include "PhysicsDynamicRigidBody.h"

#include "ThrowException.h"
#include "PhysXCoordHelpers.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"

#include "RigidBodySceneData.h"

#include <PxRigidDynamic.h>

namespace
{
	Storm::UniquePointer<physx::PxRigidDynamic> createDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData)
	{
		auto &physicsHandler = Storm::PhysicsManager::instance().getPhysXHandler();
		return physicsHandler.createDynamicRigidBody(rbSceneData);
	}
}

Storm::PhysicsDynamicRigidBody::PhysicsDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices) :
	Storm::PhysicalShape{ rbSceneData, vertices },
	_internalRb{ createDynamicRigidBody(rbSceneData) }
{
	if (!_internalRb)
	{
		Storm::throwException<std::exception>("PhysX failed to create the internal rigid body for object " + std::to_string(rbSceneData._rigidBodyID));
	}

	if (_internalRbShape && !_internalRb->attachShape(*_internalRbShape))
	{
		Storm::throwException<std::exception>("We failed to attach the created shape to the rigid body " + std::to_string(rbSceneData._rigidBodyID));
	}
}

void Storm::PhysicsDynamicRigidBody::getMeshTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const
{
	physx::PxTransform currentTransform;

	{
		std::lock_guard<std::mutex> lock{ Storm::PhysicsManager::instance()._simulationMutex };
		currentTransform = _internalRb->getGlobalPose();
	}

	outTrans = Storm::convertToStorm(currentTransform.p);
	outRot = Storm::convertToStorm<Storm::Vector3>(currentTransform.q);
}
