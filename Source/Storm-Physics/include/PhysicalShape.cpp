#include "PhysicalShape.h"

#include "SceneRigidBodyConfig.h"
#include "CollisionType.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"


namespace
{
	std::string makePhysicsName(const Storm::SceneRigidBodyConfig &rbSceneConfig)
	{
		std::string result;
		result.reserve(128);

		result += "Rb";

		if (rbSceneConfig._static)
		{
			result += "Static_";
		}
		else
		{
			result += "Dynamic_";
		}

		if (rbSceneConfig._isWall)
		{
			result += "Wall_";
		}

		switch (rbSceneConfig._collisionShape)
		{
		case Storm::CollisionType::Sphere: result += "Sphere_"; break;
		case Storm::CollisionType::Cube: result += "Cube_"; break;
		case Storm::CollisionType::IndividualParticle: result += "IndividualParticle_"; break;
		case Storm::CollisionType::Custom: result += "Custom_"; break;

		case Storm::CollisionType::None:
		default: result += "Unknown_";
		}

		result += "id";
		result += Storm::toStdString(rbSceneConfig._rigidBodyID);
		result += "_layerC";
		result += Storm::toStdString(rbSceneConfig._layerCount);

		return result;
	}
}

Storm::PhysicalShape::PhysicalShape(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes) :
	_id{ rbSceneConfig._rigidBodyID },
	_physicsName{ makePhysicsName(rbSceneConfig) }
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
