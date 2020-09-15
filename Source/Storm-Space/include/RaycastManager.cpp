#include "RaycastManager.h"

#include "ThrowException.h"


Storm::RaycastManager::RaycastManager() = default;
Storm::RaycastManager::~RaycastManager() = default;

void Storm::RaycastManager::queryRayCast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, const std::vector<Storm::PartitionSelection> &hitFlag, HitResponseCallback callback) const
{
	STORM_NOT_IMPLEMENTED;
}

void Storm::RaycastManager::queryRayCast(const Storm::Vector2 &pixelScreenPos, const std::vector<Storm::PartitionSelection> &hitFlag, HitResponseCallback callback) const
{
	STORM_NOT_IMPLEMENTED;
}

