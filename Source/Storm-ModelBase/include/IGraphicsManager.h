#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class GraphicsAction;

	class IGraphicsManager : public Storm::ISingletonHeldInterface<IGraphicsManager>
	{
	public:
		virtual ~IGraphicsManager() = default;

	public:
		virtual void update() = 0;

	public:
		virtual void executeActionAsync(GraphicsAction actionToExecute) = 0;
	};
}
