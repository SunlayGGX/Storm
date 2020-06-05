#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class GraphicsAction;
	class IRigidBody;

	class IGraphicsManager : public Storm::ISingletonHeldInterface<IGraphicsManager>
	{
	public:
		virtual ~IGraphicsManager() = default;

	public:
		virtual void update() = 0;

	public:
		virtual void executeActionAsync(GraphicsAction actionToExecute) = 0;

	public:
		virtual void addMesh(unsigned int meshId, const std::vector<Storm::Vector3> &vertexes, const std::vector<Storm::Vector3> &normals, const std::vector<unsigned int> &indexes) = 0;
		virtual void bindParentRbToMesh(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb) = 0;
	};
}
