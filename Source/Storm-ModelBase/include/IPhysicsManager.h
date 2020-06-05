#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct RigidBodySceneData;
	class IRigidBody;

	class IPhysicsManager : public Storm::ISingletonHeldInterface<IPhysicsManager>
	{
	public:
		virtual ~IPhysicsManager() = default;

	public:
		virtual void addPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertexes) = 0;
		virtual void bindParentRbToPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::shared_ptr<Storm::IRigidBody> &parentRb) = 0;
	};
}
