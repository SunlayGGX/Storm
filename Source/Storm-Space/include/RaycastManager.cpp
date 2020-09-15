#include "RaycastManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"

#include "ThreadEnumeration.h"

#include "ThrowException.h"


Storm::RaycastManager::RaycastManager() = default;
Storm::RaycastManager::~RaycastManager() = default;

void Storm::RaycastManager::queryRayCast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, std::vector<Storm::PartitionSelection> &&hitFlag, HitResponseCallback callback) const
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, origin, direction, flag = std::move(hitFlag), cb = std::move(callback)]()
	{
		this->executeRaycast(origin, direction, flag, cb);
	});
}

void Storm::RaycastManager::queryRayCast(const Storm::Vector2 &pixelScreenPos, std::vector<Storm::PartitionSelection> &&hitFlag, HitResponseCallback callback) const
{
	STORM_NOT_IMPLEMENTED;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, flag = std::move(hitFlag), cb = std::move(callback)]() mutable
	{
		Storm::Vector3 origin;
		Storm::Vector3 direction;

		// TODO : Convert the 2D pos into a 3D pos and direction using the Camera present in the graphic module.

		this->queryRayCast(origin, direction, std::move(flag), std::move(cb));
	});
}

void Storm::RaycastManager::executeRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, const std::vector<Storm::PartitionSelection> &hitFlag, const HitResponseCallback &callback) const
{
	STORM_NOT_IMPLEMENTED;
}
