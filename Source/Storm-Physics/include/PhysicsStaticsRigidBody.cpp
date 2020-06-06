#include "PhysicsStaticsRigidBody.h"

#include "ThrowException.h"

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
	Storm::PhysicalShape{ rbSceneData, vertices },
	_internalRb{ createStaticRigidBody(rbSceneData) },
	_trans{ rbSceneData._translation },
	_rotation{ rbSceneData._rotation }
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

void Storm::PhysicsStaticsRigidBody::getMeshTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const
{
	outTrans = _trans;
	outRot = _rotation;
}
