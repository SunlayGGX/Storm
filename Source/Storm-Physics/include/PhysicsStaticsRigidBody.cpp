#include "PhysicsStaticsRigidBody.h"

#include "PhysXCoordHelpers.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"

#include "SceneRigidBodyConfig.h"

#include <PxRigidStatic.h>

namespace
{
	Storm::UniquePointer<physx::PxRigidStatic> createStaticRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig)
	{
		auto &physicsHandler = Storm::PhysicsManager::instance().getPhysXHandler();
		return physicsHandler.createStaticRigidBody(rbSceneConfig);
	}
}

Storm::PhysicsStaticsRigidBody::PhysicsStaticsRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes) :
	Storm::PhysicalShape{ rbSceneConfig, vertices, indexes },
	_internalRb{ createStaticRigidBody(rbSceneConfig) },
	_trans{ rbSceneConfig._translation },
	_eulerRotation{ rbSceneConfig._rotation }
{
	if (!_internalRb)
	{
		Storm::throwException<Storm::Exception>("PhysX failed to create the internal rigid body for object " + std::to_string(rbSceneConfig._rigidBodyID));
	}

	if (_internalRbShape && !_internalRb->attachShape(*_internalRbShape))
	{
		Storm::throwException<Storm::Exception>("We failed to attach the created shape to the rigid body " + std::to_string(rbSceneConfig._rigidBodyID));
	}

	const physx::PxQuat physXQuatRot = Storm::convertToPxRotation(rbSceneConfig._rotation);
	_quatRotation = Storm::convertToStorm<Storm::Quaternion>(physXQuatRot);
}

void Storm::PhysicsStaticsRigidBody::onIterationStart() noexcept
{

}

void Storm::PhysicsStaticsRigidBody::onPostUpdate(const float) noexcept
{

}

void Storm::PhysicsStaticsRigidBody::getMeshTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const
{
	outTrans = _trans;
	outRot = _eulerRotation;
}

physx::PxRigidStatic* Storm::PhysicsStaticsRigidBody::getInternalPhysicsPointer() const
{
	return _internalRb.get();
}

Storm::Vector3 Storm::PhysicsStaticsRigidBody::getPhysicAppliedForce() const noexcept
{
	return Storm::Vector3::Zero();
}

Storm::Vector3 Storm::PhysicsStaticsRigidBody::getTotalForce(const float) const noexcept
{
	return Storm::Vector3::Zero();
}

void Storm::PhysicsStaticsRigidBody::getMeshTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const
{
	outTrans = _trans;
	outQuatRot = _quatRotation;
}
