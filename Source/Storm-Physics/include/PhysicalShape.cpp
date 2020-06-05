#include "PhysicalShape.h"

#include "RigidBodySceneData.h"
#include "CollisionType.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"

#include "ThrowException.h"


Storm::PhysicalShape::PhysicalShape(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices)
{
	auto &physicsHandler = Storm::PhysicsManager::instance().getPhysXHandler();
	if (rbSceneData._collisionShape != Storm::CollisionType::None)
	{
		_internalRbMaterial = physicsHandler.createRigidBodyMaterial(rbSceneData);
		if (!_internalRbMaterial)
		{
			Storm::throwException<std::exception>("PhysX failed to create the internal rigid body material for object " + std::to_string(rbSceneData._rigidBodyID));
		}

		_internalRbShape = physicsHandler.createRigidBodyShape(rbSceneData, vertices, _internalRbMaterial.get());
		if (!_internalRbShape)
		{
			Storm::throwException<std::exception>("PhysX failed to create the internal rigid body shape for object " + std::to_string(rbSceneData._rigidBodyID));
		}
	}
}
