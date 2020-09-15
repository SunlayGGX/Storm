#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct RaycastQueryRequest;

	class IRaycastManager : public Storm::ISingletonHeldInterface<Storm::IRaycastManager>
	{
	public:
		virtual ~IRaycastManager() = default;

	public:
		virtual void queryRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, Storm::RaycastQueryRequest &&queryRequest) const = 0;
		virtual void queryRaycast(const Storm::Vector2 &pixelScreenPos, Storm::RaycastQueryRequest &&queryRequest) const = 0;
	};
}
