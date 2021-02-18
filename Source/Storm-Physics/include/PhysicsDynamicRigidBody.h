#pragma once

#include "RigidBodyHolder.h"
#include "PhysicalShape.h"

#include "UniquePointer.h"


namespace Storm
{
	struct SceneRigidBodyConfig;
	class PhysicsConstraint;

	class PhysicsDynamicRigidBody :
		public Storm::RigidBodyHolder,
		public Storm::PhysicalShape
	{
	public:
		PhysicsDynamicRigidBody(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes);

	public:
		void onIterationStart() noexcept;
		void onPostUpdate(const float currentTime) noexcept;

	public:
		void resetForce();

		void getMeshTransform(Storm::Vector3 &outTrans, Storm::Rotation &outRot) const;
		void getMeshTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const;

		void applyForce(const Storm::Vector3 &location, const Storm::Vector3 &force);
		Storm::Vector3 getPhysicAppliedForce() const noexcept;
		Storm::Vector3 getTotalForce(const float deltaTime) const noexcept;

		physx::PxRigidDynamic* getInternalPhysicsPointer() const;

		void registerConstraint(const std::shared_ptr<Storm::PhysicsConstraint> &constraint);

		void setAngularVelocityDamping(const float angularVelocityDamping);

		void setTranslationFixed(bool fixTranslation);

		void freeFromAnimation();
		bool isAnimated() const;

	private:
		Storm::UniquePointer<physx::PxRigidDynamic> _internalRb;
		std::vector<std::shared_ptr<Storm::PhysicsConstraint>> _constraints;

		physx::PxVec3 _currentIterationVelocity;
		float _currentAngularVelocityDampingCoefficient;

		physx::PxVec3 _fixedPos;
		bool _translationFixed;

		bool _isAnimated;
	};
}
