#pragma once

#include "UniquePointer.h"


namespace Storm
{
	struct SceneRigidBodyConfig;

	class PhysicalShape
	{
	protected:
		PhysicalShape(const Storm::SceneRigidBodyConfig &rbSceneConfig, const std::vector<Storm::Vector3> &vertices, const std::vector<uint32_t> &indexes);

	public:
		virtual ~PhysicalShape() = default;

	protected:
		Storm::UniquePointer<physx::PxShape> _internalRbShape;
		Storm::UniquePointer<physx::PxMaterial> _internalRbMaterial;
	};
}
