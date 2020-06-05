#pragma once

#include "UniquePointer.h"


namespace Storm
{
	struct RigidBodySceneData;

	class PhysicalShape
	{
	protected:
		PhysicalShape(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices);

	public:
		virtual ~PhysicalShape() = default;

	protected:
		Storm::UniquePointer<physx::PxShape> _internalRbShape;
		Storm::UniquePointer<physx::PxMaterial> _internalRbMaterial;
	};
}
