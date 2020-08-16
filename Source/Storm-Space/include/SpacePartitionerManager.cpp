#include "SpacePartitionerManager.h"

#include "SingletonHolder.h"
#include "IAssetLoaderManager.h"

#include "IRigidBody.h"

#include "VoxelGrid.h"
#include "PartitionSelection.h"

#include "Vector3Utils.h"


Storm::SpacePartitionerManager::SpacePartitionerManager() = default;
Storm::SpacePartitionerManager::~SpacePartitionerManager() = default;

void Storm::SpacePartitionerManager::initialize_Implementation(float partitionLength)
{
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

	_partitionLength = -1.f; // To be sure we don't set the same length.
	this->setPartitionLength(partitionLength);
}

void Storm::SpacePartitionerManager::cleanUp_Implementation()
{
	_fluidSpacePartition.reset();
	_staticRigidBodySpacePartition.reset();
	_dynamicRigidBodySpacePartition.reset();
}

void Storm::SpacePartitionerManager::partitionSpace()
{
	auto fluidSpacePartitionSrc = std::make_unique<Storm::VoxelGrid>(_upSpaceCorner, _downSpaceCorner, _partitionLength);

	// Since the space partition are the same (this is the internal referrals that differ, but since we have not added anything, those does not differ for now). 
	// Just copy the newly created space partition instead of computing everything again. 
	auto dynamicRigidBodySpacePartitionSrc = std::make_unique<Storm::VoxelGrid>(*fluidSpacePartitionSrc);
	auto staticRigidBodySpacePartitionSrc = std::make_unique<Storm::VoxelGrid>(*fluidSpacePartitionSrc);

	_fluidSpacePartition = std::move(fluidSpacePartitionSrc);
	_dynamicRigidBodySpacePartition = std::move(dynamicRigidBodySpacePartitionSrc);
	_staticRigidBodySpacePartition = std::move(staticRigidBodySpacePartitionSrc);
}

void Storm::SpacePartitionerManager::computeSpaceReordering(const std::vector<Storm::Vector3> &particlePositions, Storm::PartitionSelection modality, const unsigned int systemId)
{
	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(modality);
	spacePartition->fill(particlePositions);
}

void Storm::SpacePartitionerManager::clearSpaceReordering(Storm::PartitionSelection modality)
{
	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(modality);
	spacePartition->clear();
}

void Storm::SpacePartitionerManager::getAllBundles(const std::vector<Storm::NeighborParticleReferral>* &outContainingBundlePtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborBundle)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality) const
{
	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(modality);
	spacePartition->getVoxelsDataAtPosition(outContainingBundlePtr, outNeighborBundle, particlePosition);
}

float Storm::SpacePartitionerManager::getPartitionLength() const
{
	return _partitionLength;
}

void Storm::SpacePartitionerManager::setPartitionLength(float length)
{
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
