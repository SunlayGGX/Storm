#include "RaycastManager.h"

#include "SingletonHolder.h"
#include "IThreadManager.h"
#include "IGraphicsManager.h"
#include "ISimulatorManager.h"
#include "IConfigManager.h"

#include "SpacePartitionerManager.h"

#include "Voxel.h"
#include "VoxelGrid.h"
#include "NeighborParticleReferral.h"

#include "RaycastQueryRequest.h"
#include "RaycastHitResult.h"

#include "SceneSimulationConfig.h"

#include "ThreadEnumeration.h"
#include "ThreadingSafety.h"


namespace
{
	Storm::Vector3 raySphereCollisionHit(const Storm::Vector3 &/*rayOrigin*/, const Storm::Vector3 &/*rayDirection*/, const Storm::Vector3 &/*particleCenter*/, const float /*particleRadiusSquared*/)
	{
		STORM_NOT_IMPLEMENTED;
	}
}


Storm::RaycastManager::RaycastManager() = default;
Storm::RaycastManager::~RaycastManager() = default;

void Storm::RaycastManager::queryRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, Storm::RaycastQueryRequest &&queryRequest) const
{
	LOG_DEBUG << "Raycast query at origin " << origin << " and direction " << direction;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, origin, direction, queryReq = std::move(queryRequest)]()
	{
		this->executeRaycast(origin, direction, queryReq);
	});
}

void Storm::RaycastManager::queryRaycast(const Storm::Vector2 &pixelScreenPos, Storm::RaycastQueryRequest &&queryRequest) const
{
	LOG_DEBUG << "Raycast query at screen position : " << pixelScreenPos;

	// Since this part is queried inside the graphic thread, we can access the Camera data without locking.
	// The downside is that the raycast query is async and will answer some frame later, like many other engine implementation.
	// The raycast query is so rare that it isn't worth to lock objects that shouldn't be shared from thread to thread at normal time...
	// Therefore, we accept the downside of such a query.

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [this, pixelScreenPos, queryReq = std::move(queryRequest), &singletonHolder]() mutable
	{
		const Storm::IGraphicsManager &graphicMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();

		if (queryReq._considerOnlyVisible)
		{
			if (queryReq._wantOnlyFirstHit)
			{
				// Heck, this problem is simpler if we only want the first hit of visible stuff only... Just use the Z-buffer.

				const Storm::Vector3 hitPosition = graphicMgr.get3DPosOfScreenPixel(pixelScreenPos);
				this->searchForNearestParticle(hitPosition, std::move(queryReq));

				return;
			}
			else
			{
				float zNear;
				float zFar;

				graphicMgr.getClippingPlaneValues(zNear, zFar);

				// The farthest from the camera will be the min distance we want to clip the results.
				if (queryReq._minDistance < zNear)
				{
					queryReq._minDistance = zNear;
				}

				// The nearest from the camera will be the max distance we want to clip the results.
				if (queryReq._maxDistance > zFar)
				{
					queryReq._maxDistance = zFar;
				}
			}
		}

		Storm::Vector3 origin;
		Storm::Vector3 direction;

		graphicMgr.convertScreenPositionToRay(pixelScreenPos, origin, direction);
		this->queryRaycast(origin, direction, std::move(queryReq));
	});
}

void Storm::RaycastManager::executeRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, const Storm::RaycastQueryRequest &queryRequest) const
{
	assert(Storm::isSpaceThread() && "this method should only be executed on the same thread that the space partitioner.");

	if (queryRequest._minDistance < 0.f)
	{
		Storm::throwException<Storm::Exception>("min distance (" + std::to_string(queryRequest._minDistance) + ") raycast shouldn't be less than 0!");
	}
	else if (queryRequest._minDistance >= queryRequest._maxDistance)
	{
		Storm::throwException<Storm::Exception>("min distance (" + std::to_string(queryRequest._minDistance) + ") raycast shouldn't be greater than max distance (" + std::to_string(queryRequest._maxDistance) + ")!");
	}

	std::vector<Storm::RaycastHitResult> results;
	results.reserve(queryRequest._particleSystemSelectionFlag.size() * 124);

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::ISimulatorManager &simulatorMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::SpacePartitionerManager &spacePartitionMgr = Storm::SpacePartitionerManager::instance();

	const float voxelLength = spacePartitionMgr.getPartitionLength();
	const float particleRadius = configMgr.getSceneSimulationConfig()._particleRadius;
	const float particleRadiusSquared = particleRadius * particleRadius;
	for (const Storm::PartitionSelection selection : queryRequest._particleSystemSelectionFlag)
	{
		const auto &partition = spacePartitionMgr.getSpacePartition(selection);
		
		std::vector<Storm::Voxel*> voxelsUnderRaycast = partition->getVoxelsUnderRaycast(origin, direction, queryRequest._minDistance, queryRequest._maxDistance, voxelLength);

		const float minDistSquared = queryRequest._minDistance * queryRequest._minDistance;
		const float maxDistSquared = queryRequest._maxDistance * queryRequest._maxDistance;

		for (const Storm::Voxel* voxelPtr : voxelsUnderRaycast)
		{
			const Storm::Voxel &voxel = *voxelPtr;

			const std::vector<Storm::Vector3>* particleSystemPositions = nullptr;
			unsigned int lastId = std::numeric_limits<decltype(lastId)>::max();

			const auto &voxelData = voxel.getData();
			for (const auto &particleReferral : voxelData)
			{
				if (lastId != particleReferral._systemId)
				{
					lastId = particleReferral._systemId;
					particleSystemPositions = &simulatorMgr.getParticleSystemPositionsReferences(particleReferral._systemId);
				}

				 const Storm::Vector3 &particleCenter = (*particleSystemPositions)[particleReferral._particleIndex];

				 Storm::Vector3 hitPosition = raySphereCollisionHit(origin, direction, particleCenter, particleRadiusSquared);
				 const float distCollisionSquared = (hitPosition - origin).squaredNorm();
				 if (distCollisionSquared > minDistSquared && distCollisionSquared < maxDistSquared)
				 {
					 results.emplace_back(particleReferral._particleIndex, particleReferral._systemId, hitPosition);
				 }
			}
		}
	}

	queryRequest._hitResponseCallback(std::move(results));
}

void Storm::RaycastManager::searchForNearestParticle(const Storm::Vector3 &position, Storm::RaycastQueryRequest &&queryRequest) const
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	singletonHolder.getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, position, queryReq = std::move(queryRequest)]()
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		const Storm::ISimulatorManager &simulatorMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();

		float minDistSquaredFound = std::numeric_limits<float>::max();
		const Storm::NeighborParticleReferral* resultReferral = nullptr;

		const Storm::SpacePartitionerManager &spaceMgr = Storm::SpacePartitionerManager::instance();
		for (const Storm::PartitionSelection modality : queryReq._particleSystemSelectionFlag)
		{
			const std::vector<Storm::NeighborParticleReferral>* containingBundlePtr = nullptr;
			const std::vector<Storm::NeighborParticleReferral>* neighborhoodBundles[Storm::k_neighborLinkedBunkCount];

			spaceMgr.getAllBundles(containingBundlePtr, neighborhoodBundles, position, modality);

			const std::vector<Storm::Vector3>* particleSystemPositions = nullptr;
			unsigned int lastId = std::numeric_limits<decltype(lastId)>::max();

#if STORM_USE_INTRINSICS
			const __m128 positionAlias = _mm_loadu_ps(reinterpret_cast<const float*>(&position[0]));
#else
			const Storm::Vector3 &positionAlias = position;
#endif

			const auto searchLambda = [&particleSystemPositions, &lastId, &simulatorMgr, &positionAlias, &minDistSquaredFound, &resultReferral](const Storm::NeighborParticleReferral &particleReferral)
			{
				if (lastId != particleReferral._systemId)
				{
					lastId = particleReferral._systemId;
					particleSystemPositions = &simulatorMgr.getParticleSystemPositionsReferences(particleReferral._systemId);
				}

#if STORM_USE_INTRINSICS

				const __m128 particleCenter = _mm_loadu_ps(reinterpret_cast<const float*>(&(*particleSystemPositions)[particleReferral._particleIndex][0]));

				enum : int
				{
					// Masks are for those component, in this order : wzyx

					broadcastMask = 0b0001,
					conditionMask = 0b0111,

					dotProductMask = conditionMask << 4 | broadcastMask
				};

				const __m128 outPosDiff = _mm_sub_ps(particleCenter, positionAlias);
				const float normSquared = _mm_dp_ps(outPosDiff, outPosDiff, dotProductMask).m128_f32[0];
				if (normSquared < minDistSquaredFound)
				{
					minDistSquaredFound = normSquared;
					resultReferral = &particleReferral;
				}
#else
				const Storm::Vector3 &particleCenter = (*particleSystemPositions)[particleReferral._particleIndex];

				float tmp = particleCenter.x() - positionAlias.x();
				float distSquared = tmp * tmp;
				if (distSquared < minDistSquaredFound)
				{
					tmp = particleCenter.y() - positionAlias.y();
					distSquared += tmp * tmp;
					if (distSquared < minDistSquaredFound)
					{
						tmp = particleCenter.z() - positionAlias.z();
						distSquared += tmp * tmp;
						if (distSquared < minDistSquaredFound)
						{
							minDistSquaredFound = distSquared;
							resultReferral = &particleReferral;
						}
					}
				}
#endif
			};

			for (const Storm::NeighborParticleReferral &particleReferral : *containingBundlePtr)
			{
				searchLambda(particleReferral);
			}

			for (const std::vector<Storm::NeighborParticleReferral>*const* neighborBundle = neighborhoodBundles; *neighborBundle != nullptr; ++neighborBundle)
			{
				for (const Storm::NeighborParticleReferral &particleReferral : **neighborBundle)
				{
					searchLambda(particleReferral);
				}
			}
		}

		std::vector<Storm::RaycastHitResult> result;

		if (resultReferral != nullptr)
		{
			result.emplace_back(resultReferral->_particleIndex, resultReferral->_systemId, position);
		}

		queryReq._hitResponseCallback(std::move(result));
	});
}
