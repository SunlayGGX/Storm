#pragma once

#include "RigidBodyHolder.h"
#include "PhysicalShape.h"

#include "UniquePointer.h"


namespace Storm
{
	struct RigidBodySceneData;

	class PhysicsDynamicRigidBody :
		public Storm::RigidBodyHolder,
		public Storm::PhysicalShape
	{
	public:
		PhysicsDynamicRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices);

	public:
		void getMeshTransform(Storm::Vector3 &outTrans, Storm::Vector3 &outRot) const;
		void getMeshTransform(Storm::Vector3 &outTrans, Storm::Quaternion &outQuatRot) const;

	private:
		Storm::UniquePointer<physx::PxRigidDynamic> _internalRb;
	};
}
