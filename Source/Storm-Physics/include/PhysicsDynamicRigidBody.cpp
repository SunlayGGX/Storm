#include "PhysicsDynamicRigidBody.h"

#include "ThrowException.h"
#include "PhysXCoordHelpers.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"

#include "RigidBodySceneData.h"

#include "PhysicsConstraint.h"

#include <PxRigidDynamic.h>


namespace
{
	Storm::UniquePointer<physx::PxRigidDynamic> createDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData)
	{
		auto &physicsHandler = Storm::PhysicsManager::instance().getPhysXHandler();
		return physicsHandler.createDynamicRigidBody(rbSceneData);
	}

	template<class RbType>
	physx::PxTransform retrieveCurrentTransform(const RbType &rb)
	{
		std::lock_guard<std::mutex> lock{ Storm::PhysicsManager::instance()._simulationMutex };
		return rb->getGlobalPose();
	}
}

Storm::PhysicsDynamicRigidBody::PhysicsDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes) :
	Storm::PhysicalShape{ rbSceneData, vertices, indexes },
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

void Storm::PhysicsDynamicRigidBody::resetForce()
{
	const physx::PxVec3 zeroVec{ 0.f, 0.f, 0.f };
	_internalRb->setForceAndTorque(zeroVec, zeroVec);
}

void Storm::PhysicsDynamicRigidBody::getMeshTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const
{
	const physx::PxTransform currentTransform = retrieveCurrentTransform(_internalRb);

	outTrans = Storm::convertToStorm(currentTransform.p);
	outRot = Storm::convertToStorm<Storm::Vector3>(currentTransform.q);
}

void Storm::PhysicsDynamicRigidBody::applyForce(const Storm::Vector3 &location, const Storm::Vector3 &force)
{
	physx::PxRigidBodyExt::addForceAtLocalPos(*_internalRb, Storm::convertToPx(force), Storm::convertToPx(location));
}

Storm::Vector3 Storm::PhysicsDynamicRigidBody::getAppliedForce() const noexcept
{
	Storm::Vector3 force = _internalRb->getMass() * Storm::PhysicsManager::instance().getPhysXHandler().getGravity();

	for (const auto &constraint : _constraints)
	{
		force += constraint->getForceApplied();
	}

	return force;
}

physx::PxRigidDynamic* Storm::PhysicsDynamicRigidBody::getInternalPhysicsPointer() const
{
	return _internalRb.get();
}

void Storm::PhysicsDynamicRigidBody::getMeshTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const
{
	physx::PxTransform currentTransform;

	{
		std::lock_guard<std::mutex> lock{ Storm::PhysicsManager::instance()._simulationMutex };
		currentTransform = _internalRb->getGlobalPose();
	}

	outTrans = Storm::convertToStorm(currentTransform.p);
	outQuatRot = Storm::convertToStorm<Storm::Quaternion>(currentTransform.q);
}

void Storm::PhysicsDynamicRigidBody::registerConstraint(const std::shared_ptr<Storm::PhysicsConstraint> &constraint)
{
	_constraints.emplace_back(constraint);
}
