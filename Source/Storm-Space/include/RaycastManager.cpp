#include "RaycastManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "IGraphicsManager.h"

#include "ThreadEnumeration.h"

#include "RaycastQueryRequest.h"

#include "ThrowException.h"


Storm::RaycastManager::RaycastManager() = default;
Storm::RaycastManager::~RaycastManager() = default;

void Storm::RaycastManager::queryRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, Storm::RaycastQueryRequest &&queryRequest) const
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, origin, direction, queryReq = std::move(queryRequest)]()
	{
		this->executeRaycast(origin, direction, queryReq);
	});
}

void Storm::RaycastManager::queryRaycast(const Storm::Vector2 &pixelScreenPos, Storm::RaycastQueryRequest &&queryRequest) const
{
	// Since this part is queried inside the graphic thread, we can access the Camera data without locking.
	// The downside is that the raycast query is async and will answer some frame later, like many other engine implementation.
	// The raycast query is so rare that it isn't worth to lock objects that shouldn't be shared from thread to thread at normal time...
	// Therefore, we accept the downside of such a query.

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, pixelScreenPos, queryReq = std::move(queryRequest), &singletonHolder]() mutable
	{
		Storm::Vector3 origin;
		Storm::Vector3 direction;

		const Storm::IGraphicsManager &graphicMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
		graphicMgr.convertScreenPositionToRay(pixelScreenPos, origin, direction);

		this->queryRaycast(origin, direction, std::move(queryReq));
	});
}

void Storm::RaycastManager::executeRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, const Storm::RaycastQueryRequest &queryRequest) const
{
	STORM_NOT_IMPLEMENTED;


}
