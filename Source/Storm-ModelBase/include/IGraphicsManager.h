#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IRigidBody;

	class IGraphicsManager : public Storm::ISingletonHeldInterface<IGraphicsManager>
	{
	public:
		virtual ~IGraphicsManager() = default;

	public:
		virtual void update() = 0;

	public:
		virtual void addMesh(unsigned int meshId, const std::vector<Storm::Vector3> &vertexes, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes) = 0;
		virtual void bindParentRbToMesh(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb) const = 0;

	public:
		// Warning : the caller makes the copy (a copy of the data will be made). It cannot be avoided to prevent data races but this is the most efficient way I thought...
		// The caller (Simulation thread) will copy the data it has and push it by move to a staging location waiting for the graphic thread to retrieve it.
		// When the Graphic thread begins its loop, it will move the staging data into its own data (no copy) and continue with it. No lock while copying is made so no bottleneck.
		virtual void pushParticlesData(unsigned int particleSystemId, const std::vector<Storm::Vector3> &particlePosData, bool isFluids, bool isWall) = 0;
	};
}
