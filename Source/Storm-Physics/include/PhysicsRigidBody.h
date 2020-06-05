#pragma once

#include "RigidBodyHolder.h"


namespace Storm
{
	struct RigidBodySceneData;

	class PhysicsRigidBody : public Storm::RigidBodyHolder
	{
	public:
		PhysicsRigidBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertices);
	};
}
