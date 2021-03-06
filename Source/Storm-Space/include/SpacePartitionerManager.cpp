#include "SpacePartitionerManager.h"

#include "SingletonHolder.h"
#include "IAssetLoaderManager.h"

#include "IRigidBody.h"

#include "VoxelGrid.h"
#include "PartitionSelection.h"
#include "DistanceSpacePartitionProxy.h"

#include "Vector3Utils.h"

#include "ThreadingSafety.h"


Storm::SpacePartitionerManager::SpacePartitionerManager() = default;
Storm::SpacePartitionerManager::~SpacePartitionerManager() = default;

void Storm::SpacePartitionerManager::initialize_Implementation(float partitionLength)
{
	LOG_COMMENT << "Starting to initialize the Space partitioner manager";

	_downSpaceCorner = Storm::Vector3{
		std::numeric_limits<Storm::Vector3::Scalar>::max(),
		std::numeric_limits<Storm::Vector3::Scalar>::max(),
		std::numeric_limits<Storm::Vector3::Scalar>::max()
	};

	_upSpaceCorner = Storm::Vector3{
		std::numeric_limits<Storm::Vector3::Scalar>::lowest(),
		std::numeric_limits<Storm::Vector3::Scalar>::lowest(),
		std::numeric_limits<Storm::Vector3::Scalar>::lowest()
	};

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IAssetLoaderManager &assetLoaderMgr = singletonHolder.getSingleton<Storm::IAssetLoaderManager>();
	const auto &allRigidBodies = assetLoaderMgr.getRigidBodyArray();
	for (const auto &rigidBody : allRigidBodies)
	{
		const std::vector<Storm::Vector3> &allVertexes = rigidBody->getRigidBodyParticlesWorldPositions();
		for (const Storm::Vector3 &vertex : allVertexes)
		{
			Storm::minMaxInPlace(_downSpaceCorner, _upSpaceCorner, vertex, [](auto &vect) -> auto& { return vect.x(); });
			Storm::minMaxInPlace(_downSpaceCorner, _upSpaceCorner, vertex, [](auto &vect) -> auto& { return vect.y(); });
			Storm::minMaxInPlace(_downSpaceCorner, _upSpaceCorner, vertex, [](auto &vect) -> auto& { return vect.z(); });
		}
	}

	Storm::minNegativeInPlaceFromBoth(_gridShiftOffset, _downSpaceCorner, _upSpaceCorner, [](auto &vect) -> auto& { return vect.x(); });
	Storm::minNegativeInPlaceFromBoth(_gridShiftOffset, _downSpaceCorner, _upSpaceCorner, [](auto &vect) -> auto& { return vect.y(); });
	Storm::minNegativeInPlaceFromBoth(_gridShiftOffset, _downSpaceCorner, _upSpaceCorner, [](auto &vect) -> auto& { return vect.z(); });

	_partitionLength = -1.f; // To be sure we don't set the same length.
	this->setPartitionLength(partitionLength);

	LOG_COMMENT << "Space partitioner manager init finished";
}

void Storm::SpacePartitionerManager::cleanUp_Implementation()
{
	assert(Storm::isSpaceThread() && "This method should only be executed inside the space thread!");

	LOG_COMMENT << "Space partitioner manager cleanUp starting";

	_fluidSpacePartition.reset();
	_staticRigidBodySpacePartition.reset();
	_dynamicRigidBodySpacePartition.reset();

	LOG_COMMENT << "Space partitioner manager cleanUp finished";
}

void Storm::SpacePartitionerManager::partitionSpace()
{
	assert(Storm::isSpaceThread() && "This method should only be executed inside the space thread!");

	LOG_DEBUG << "Space partitioning requested from " << _upSpaceCorner << " to " << _downSpaceCorner << " with a partition length of " << _partitionLength;

	auto fluidSpacePartitionSrc = std::make_unique<Storm::VoxelGrid>(_upSpaceCorner, _downSpaceCorner, _partitionLength);

	// Since the space partition are the same (this is the internal referrals that differ, but since we have not added anything, those does not differ for now). 
	// Just copy the newly created space partition instead of computing everything again. 
	auto dynamicRigidBodySpacePartitionSrc = std::make_unique<Storm::VoxelGrid>(*fluidSpacePartitionSrc);
	auto staticRigidBodySpacePartitionSrc = std::make_unique<Storm::VoxelGrid>(*fluidSpacePartitionSrc);

	_fluidSpacePartition = std::move(fluidSpacePartitionSrc);
	_dynamicRigidBodySpacePartition = std::move(dynamicRigidBodySpacePartitionSrc);
	_staticRigidBodySpacePartition = std::move(staticRigidBodySpacePartitionSrc);
}

void Storm::SpacePartitionerManager::clearSpaceReorderingNoStatic()
{
	this->clearSpaceReorderingForPartition(Storm::PartitionSelection::Fluid);
	this->clearSpaceReorderingForPartition(Storm::PartitionSelection::DynamicRigidBody);
}

void Storm::SpacePartitionerManager::computeSpaceReordering(const std::vector<Storm::Vector3> &particlePositions, Storm::PartitionSelection modality, const unsigned int systemId)
{
	assert(Storm::isSpaceThread() && "This method should only be executed inside the space thread!");

	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(modality);
	spacePartition->fill(this->getPartitionLength(), _gridShiftOffset, particlePositions, systemId);
}

void Storm::SpacePartitionerManager::clearSpaceReorderingForPartition(Storm::PartitionSelection modality)
{
	assert(Storm::isSpaceThread() && "This method should only be executed inside the space thread!");

	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(modality);
	spacePartition->clear();
}

void Storm::SpacePartitionerManager::getAllBundles(const std::vector<Storm::NeighborParticleReferral>* &outContainingBundlePtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborBundle)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality) const
{
	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(modality);
	spacePartition->getVoxelsDataAtPosition(this->getPartitionLength(), _gridShiftOffset, outContainingBundlePtr, outNeighborBundle, particlePosition);
}

void Storm::SpacePartitionerManager::getContainingBundle(const std::vector<Storm::NeighborParticleReferral>* &outContainingBundlePtr, const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality) const
{
	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(modality);
	spacePartition->getVoxelsDataAtPosition(this->getPartitionLength(), _gridShiftOffset, outContainingBundlePtr, particlePosition);
}

float Storm::SpacePartitionerManager::getPartitionLength() const
{
	return _partitionLength;
}

void Storm::SpacePartitionerManager::setPartitionLength(float length)
{
	assert(Storm::isSpaceThread() && "This method should only be executed inside the space thread!");

	if (_partitionLength != length)
	{
		_partitionLength = length;

		// Recreate all partitions.
		this->partitionSpace();
	}
}

const std::unique_ptr<Storm::VoxelGrid>& Storm::SpacePartitionerManager::getSpacePartition(Storm::PartitionSelection modality) const
{
	switch (modality)
	{
	case Storm::PartitionSelection::Fluid: return _fluidSpacePartition;
	case Storm::PartitionSelection::DynamicRigidBody: return _dynamicRigidBodySpacePartition;
	case Storm::PartitionSelection::StaticRigidBody: return _staticRigidBodySpacePartition;

	default:
		assert(false && "Space partition referred to the PartitionSelection enum value doesn't exist!");
		__assume(false);
		return nullptr;
	}
}

std::shared_ptr<Storm::IDistanceSpacePartitionProxy> Storm::SpacePartitionerManager::makeDistancePartitionProxy(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, const float partitionLength)
{
	std::shared_ptr<Storm::IDistanceSpacePartitionProxy> result = std::make_shared<Storm::DistanceSpacePartitionProxy>(upCorner, downCorner, partitionLength);
	return result;
}
