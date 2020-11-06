#pragma once

#include "RigidBodyHolder.h"
#include "PhysicalShape.h"

#include "UniquePointer.h"


namespace Storm
{
	struct RigidBodySceneData;

	class PhysicsStaticsRigidBody :
		public Storm::RigidBodyHolder,
		public Storm::PhysicalShape
	{
	public:
		PhysicsStaticsRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes);

	public:
		void getMeshTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const;
		void getMeshTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const;

		physx::PxRigidStatic* getInternalPhysicsPointer() const;

		Storm::Vector3 getAppliedForce() const noexcept;

	private:
		Storm::UniquePointer<physx::PxRigidStatic> _internalRb;

		Storm::Vector3 _trans;
		Storm::Vector3 _eulerRotation;
		Storm::Quaternion _quatRotation;
	};
}
