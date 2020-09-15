#pragma once

#include "Singleton.h"
#include "SingletonDefaultImplementation.h"

#include "IRaycastManager.h"


namespace Storm
{
	class RaycastManager :
		private Storm::Singleton<Storm::RaycastManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IRaycastManager
	{
		STORM_DECLARE_SINGLETON(RaycastManager);

	public:
		void queryRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, Storm::RaycastQueryRequest &&queryRequest) const final override;
		void queryRaycast(const Storm::Vector2 &pixelScreenPos, Storm::RaycastQueryRequest &&queryRequest) const override;

	private:
		void executeRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, const Storm::RaycastQueryRequest &queryRequest) const;
	};
}
