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
		void queryRayCast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, std::vector<Storm::PartitionSelection> &&hitFlag, HitResponseCallback callback) const final override;
		void queryRayCast(const Storm::Vector2 &pixelScreenPos, std::vector<Storm::PartitionSelection> &&hitFlag, HitResponseCallback callback) const override;

	private:
		void executeRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, const std::vector<Storm::PartitionSelection> &hitFlag, const HitResponseCallback &callback) const;
	};
}
