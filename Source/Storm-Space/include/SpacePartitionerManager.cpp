#include "SpacePartitionerManager.h"

#include "SingletonHolder.h"
#include "IAssetLoaderManager.h"

#include "IRigidBody.h"

#include "VoxelGrid.h"

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
		const std::vector<Storm::Vector3> &allVertexes = rigidBody->getRigidBodyObjectSpaceVertexes();
		for (const Storm::Vector3 &vertex : allVertexes)
		{
			Storm::minMaxInPlace(_downSpaceCorner, _upSpaceCorner, vertex, [](auto &vect) -> auto& { return vect.x(); });
			Storm::minMaxInPlace(_downSpaceCorner, _upSpaceCorner, vertex, [](auto &vect) -> auto& { return vect.y(); });
			Storm::minMaxInPlace(_downSpaceCorner, _upSpaceCorner, vertex, [](auto &vect) -> auto& { return vect.z(); });
		}
	}

	this->setPartitionLength(partitionLength);
}

void Storm::SpacePartitionerManager::cleanUp_Implementation()
{
	_fluidSpacePartition.clear();
	_rigidBodySpacePartition.clear();
}

void Storm::SpacePartitionerManager::partitionSpace(unsigned int systemId, bool isFluid)
{
	this->partitionSpace_Internal(isFluid ? _fluidSpacePartition : _rigidBodySpacePartition, systemId);
}

void Storm::SpacePartitionerManager::computeSpaceReordering(const std::vector<Storm::Vector3> &particlePositions, unsigned int systemId, bool isFluid)
{
	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(systemId, isFluid);
	spacePartition->clear();

	spacePartition->fill(particlePositions);
}

void Storm::SpacePartitionerManager::getAllBundles(const std::vector<std::size_t>* &outContainingBundlePtr, std::vector<const std::vector<std::size_t> *> &outNeighborBundle, const Storm::Vector3 &particlePosition, unsigned int systemId, bool isFluid) const
{
	const std::unique_ptr<Storm::VoxelGrid> &spacePartition = this->getSpacePartition(systemId, isFluid);
	spacePartition->getVoxelsDataAtPosition(outContainingBundlePtr, outNeighborBundle, particlePosition);
}

float Storm::SpacePartitionerManager::getPartitionLength() const
{
	return _partitionLength;
}

void Storm::SpacePartitionerManager::setPartitionLength(float length)
{
	_partitionLength = length;
}

void Storm::SpacePartitionerManager::partitionSpace_Internal(Storm::SpacePartitionerManager::BundleMap &spacePartitionMap, unsigned int systemId)
{
	auto spacePartition = std::make_unique<Storm::VoxelGrid>(_upSpaceCorner, _downSpaceCorner, _partitionLength);
	spacePartitionMap[systemId] = std::move(spacePartition);
}

const std::unique_ptr<Storm::VoxelGrid>& Storm::SpacePartitionerManager::getSpacePartition(unsigned int systemId, bool isFluid) const
{
	if (isFluid)
	{
		if (auto found = _fluidSpacePartition.find(systemId); found != std::end(_fluidSpacePartition))
		{
			return found->second;
		}
	}
	else
	{
		if (auto found = _rigidBodySpacePartition.find(systemId); found != std::end(_rigidBodySpacePartition))
		{
			return found->second;
		}
	}

	Storm::throwException<std::exception>("Space partition " + std::to_string(systemId) + " supposed to be a " + (isFluid ? "fluid" : "rigid body") + " was not found!");
}
