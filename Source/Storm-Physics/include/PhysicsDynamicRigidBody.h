#pragma once

#include "RigidBodyHolder.h"
#include "PhysicalShape.h"

#include "UniquePointer.h"


namespace Storm
{
	struct RigidBodySceneData;
	class PhysicsConstraint;

	class PhysicsDynamicRigidBody :
		public Storm::RigidBodyHolder,
		public Storm::PhysicalShape
	{
	public:
		PhysicsDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes);

	public:
		void onIterationStart() noexcept;

	public:
		void resetForce();

		void getMeshTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const;
		void getMeshTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const;

		void applyForce(const Storm::Vector3 &location, const Storm::Vector3 &force);
		Storm::Vector3 getAppliedForce() const noexcept;

		physx::PxRigidDynamic* getInternalPhysicsPointer() const;

		void registerConstraint(const std::shared_ptr<Storm::PhysicsConstraint> &constraint);

	private:
		Storm::UniquePointer<physx::PxRigidDynamic> _internalRb;
		std::vector<std::shared_ptr<Storm::PhysicsConstraint>> _constraints;

		physx::PxVec3 _currentIterationVelocity;
	};
}
