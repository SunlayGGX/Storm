#include "PhysicalShape.h"

#include "SceneRigidBodyConfig.h"
#include "CollisionType.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"


Storm::PhysicalShape::PhysicalShape(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes)
{
	auto &physicsHandler = Storm::PhysicsManager::instance().getPhysXHandler();
	if (rbSceneConfig._collisionShape != Storm::CollisionType::None)
	{
		_internalRbMaterial = physicsHandler.createRigidBodyMaterial(rbSceneConfig);
		if (!_internalRbMaterial)
		{
			Storm::throwException<Storm::Exception>("PhysX failed to create the internal rigid body material for object " + std::to_string(rbSceneConfig._rigidBodyID));
		}

		_internalRbShape = physicsHandler.createRigidBodyShape(rbSceneConfig, vertices, indexes, _internalRbMaterial.get());
		if (!_internalRbShape)
		{
			Storm::throwException<Storm::Exception>("PhysX failed to create the internal rigid body shape for object " + std::to_string(rbSceneConfig._rigidBodyID));
		}
	}
}
