#include "PhysicsDynamicRigidBody.h"

#include "PhysXCoordHelpers.h"

#include "SingletonHolder.h"
#include "IAnimationManager.h"

#include "PhysicsManager.h"
#include "PhysXHandler.h"

#include "SceneRigidBodyConfig.h"

#include "PhysicsConstraint.h"

#include "ThreadingSafety.h"

#include <PxRigidDynamic.h>


namespace
{
	Storm::UniquePointer<physx::PxRigidDynamic> createDynamicRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig)
	{
		auto &physicsHandler = Storm::PhysicsManager::instance().getPhysXHandler();
		return physicsHandler.createDynamicRigidBody(rbSceneConfig);
	}

	template<class RbType>
	physx::PxTransform retrieveCurrentTransform(const RbType &rb)
	{
		std::lock_guard<std::mutex> lock{ Storm::PhysicsManager::instance()._simulationMutex };
		return rb->getGlobalPose();
	}

	float computeAngularVelocityDampingCoeff(const float angularVelocityDampingValue)
	{
		return 1.f - angularVelocityDampingValue;
	}
}

Storm::PhysicsDynamicRigidBody::PhysicsDynamicRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes) :
	Storm::PhysicalShape{ rbSceneConfig, vertices, indexes },
	_currentIterationVelocity{ 0.f, 0.f, 0.f },
	_internalRb{ createDynamicRigidBody(rbSceneConfig) },
	_currentAngularVelocityDampingCoefficient{ computeAngularVelocityDampingCoeff(rbSceneConfig._angularVelocityDamping) },
	_translationFixed{ rbSceneConfig._isTranslationFixed }
{
	if (!_internalRb)
	{
		Storm::throwException<Storm::Exception>("PhysX failed to create the internal rigid body for object " + std::to_string(rbSceneConfig._rigidBodyID));
	}

	if (_internalRbShape && !_internalRb->attachShape(*_internalRbShape))
	{
		Storm::throwException<Storm::Exception>("We failed to attach the created shape to the rigid body " + std::to_string(rbSceneConfig._rigidBodyID));
	}

	_internalRb->setName(_physicsName.c_str());

	if (_translationFixed)
	{
		_fixedPos = _internalRb->getGlobalPose().p;
	}

	_isAnimated = !rbSceneConfig._animationXmlContent.empty();
	_internalRb->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, _isAnimated);
}

void Storm::PhysicsDynamicRigidBody::onIterationStart() noexcept
{
	_currentIterationVelocity = _internalRb->getLinearVelocity();

	if (!_isAnimated)
	{
		_internalRb->clearForce();
		_internalRb->clearTorque();

		_totalForce.setZero();
		_totalTorque.setZero();

		if (_currentAngularVelocityDampingCoefficient != 1.f)
		{
			// PhysX angular damping coeff default value of 0.05 is a lie, therefore I'm doing the damping myself.
			physx::PxVec3 currentAngularVelocity = _internalRb->getAngularVelocity();
			currentAngularVelocity *= _currentAngularVelocityDampingCoefficient;
			_internalRb->setAngularVelocity(currentAngularVelocity);
		}
	}
}

void Storm::PhysicsDynamicRigidBody::onPreUpdate(const float deltaTime) noexcept
{
	if (!_isAnimated)
	{
		const float rbMass = _internalRb->getMass();
		if (!_translationFixed)
		{
			const Storm::Vector3 rbVelocityChange = _totalForce * (deltaTime / rbMass);
			_internalRb->setLinearVelocity(_internalRb->getLinearVelocity() + Storm::convertToPx(rbVelocityChange));
		}

#if false
		auto inertiaTensor = _internalRb->getMassSpaceInertiaTensor();
		const physx::PxVec3 rbAngularVelocityChange{
			_totalTorque.x() * deltaTime / (inertiaTensor.x * rbMass),
			_totalTorque.y() * deltaTime / (inertiaTensor.y * rbMass),
			_totalTorque.z() * deltaTime / (inertiaTensor.z * rbMass)
		};

		_internalRb->setAngularVelocity(_internalRb->getAngularVelocity() + rbAngularVelocityChange);
#else
		auto invInertiaTensor = Storm::convertToStorm(_internalRb->getMassSpaceInvInertiaTensor() * rbMass).asDiagonal();
		const Storm::Vector3 rbAngularVelocityChange = invInertiaTensor * _totalTorque * deltaTime;

		_internalRb->setAngularVelocity(_internalRb->getAngularVelocity() + Storm::convertToPx(rbAngularVelocityChange));
#endif

	}
}

void Storm::PhysicsDynamicRigidBody::onPostUpdate(const float currentTime) noexcept
{
	if (_translationFixed)
	{
		physx::PxTransform currentTransform = _internalRb->getGlobalPose();
		currentTransform.p = _fixedPos;

		_internalRb->setGlobalPose(currentTransform);

		_internalRb->setLinearVelocity(physx::PxVec3{ physx::PxZERO::PxZero });
	}

	if (_isAnimated)
	{
		Storm::Vector3 animatedPos;
		Storm::Rotation animatedRot;

		Storm::IAnimationManager &animationMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAnimationManager>();
		if (animationMgr.retrieveAnimationData(currentTime, _id, animatedPos, animatedRot))
		{
			physx::PxTransform animatedTransform = _internalRb->getGlobalPose();
			animatedTransform.p = Storm::convertToPx(animatedPos);
			animatedTransform.q = Storm::convertToPxRotation(animatedRot);

			_internalRb->setGlobalPose(animatedTransform);
		}
		else
		{
			this->freeFromAnimation();
		}
	}
}

void Storm::PhysicsDynamicRigidBody::resetForce()
{
	if (!_isAnimated)
	{
		const physx::PxVec3 zeroVec{ 0.f, 0.f, 0.f };
		_internalRb->setForceAndTorque(zeroVec, zeroVec);
	}
}

void Storm::PhysicsDynamicRigidBody::applyForce(const Storm::Vector3 &location, const Storm::Vector3 &force)
{
#if false
	physx::PxRigidBodyExt::addForceAtLocalPos(*_internalRb, Storm::convertToPx(force), Storm::convertToPx(location));
#elif false
	physx::PxRigidBodyExt::addLocalForceAtLocalPos(*_internalRb, Storm::convertToPx(force), Storm::convertToPx(location));
#elif false
	const physx::PxVec3 forcePx = Storm::convertToPx(force);
	_internalRb->addForce(forcePx);

	const Storm::Vector3 locationRel = location - this->getRbPosition();

	_internalRb->addTorque(Storm::convertToPx(locationRel.cross(force)));
#else
	_totalForce += force;

	const Storm::Vector3 locationRel = location - this->getRbPosition();
	_totalTorque += locationRel.cross(force);

#endif
}

Storm::Vector3 Storm::PhysicsDynamicRigidBody::getPhysicAppliedForce() const noexcept
{
	Storm::Vector3 force = _internalRb->getMass() * Storm::PhysicsManager::instance().getPhysXHandler().getGravity();

	for (const auto &constraint : _constraints)
	{
		force += constraint->getForceApplied();
	}

	return force;
}

Storm::Vector3 Storm::PhysicsDynamicRigidBody::getTotalForce(const float deltaTime) const noexcept
{
	// Since PhysX engine doesn't provide a way to get the force applied on this frame.
	// We need to make an invert Euler Integration from the velocity change between the iteration start and the iteration end.
	// Its means that this force is only the force applied at the end of the iteration. When the iteration started, this force is reset to { 0, 0, 0 }
	const float currentMass = _internalRb->getMass();
	const physx::PxVec3 currentVelocity = _internalRb->getLinearVelocity();
	const physx::PxVec3 currentAcceleration = (currentVelocity - _currentIterationVelocity) / deltaTime;

	return Storm::convertToStorm(currentMass * currentAcceleration);
}

Storm::Vector3 Storm::PhysicsDynamicRigidBody::getCurrentLinearVelocity() const noexcept
{
	assert(Storm::isPhysicsThread() && "This method should only be called from Physics thread.");
	return Storm::convertToStorm(_internalRb->getLinearVelocity());
}

physx::PxRigidDynamic* Storm::PhysicsDynamicRigidBody::getInternalPhysicsPointer() const
{
	return _internalRb.get();
}

void Storm::PhysicsDynamicRigidBody::getMeshTransform(Storm::Vector3 &outTrans, Storm::Rotation &outRot) const
{
	physx::PxTransform currentTransform;

	{
		std::lock_guard<std::mutex> lock{ Storm::PhysicsManager::instance()._simulationMutex };
		currentTransform = _internalRb->getGlobalPose();
	}

	outTrans = Storm::convertToStorm(currentTransform.p);
	outRot = Storm::convertToStorm<Storm::Rotation>(currentTransform.q);
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

void Storm::PhysicsDynamicRigidBody::setAngularVelocityDamping(const float angularVelocityDamping)
{
	if (angularVelocityDamping > 1.f)
	{
		Storm::throwException<Storm::Exception>("Angular velocity damping coeff cannot exceed 1.0. Value was " + std::to_string(angularVelocityDamping));
	}

	_currentAngularVelocityDampingCoefficient = computeAngularVelocityDampingCoeff(angularVelocityDamping);
}

void Storm::PhysicsDynamicRigidBody::setTranslationFixed(bool fixTranslation)
{
	_translationFixed = fixTranslation;
	if (_translationFixed)
	{
		_fixedPos = _internalRb->getGlobalPose().p;
	}
}

void Storm::PhysicsDynamicRigidBody::freeFromAnimation()
{
	_internalRb->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, false);
	_isAnimated = false;
}

bool Storm::PhysicsDynamicRigidBody::isAnimated() const
{
	return _isAnimated;
}
