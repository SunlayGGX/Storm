#pragma once

#include "SingletonHeldInterfaceBase.h"

#include "SpacePartitionConstants.h"

namespace Storm
{
	enum class PartitionSelection;
	struct NeighborParticleReferral;

	class ISpacePartitionerManager : public Storm::ISingletonHeldInterface<Storm::ISpacePartitionerManager>
	{
	public:
		virtual ~ISpacePartitionerManager() = default;

	public:
		// Partition the space into smaller chunks. This initialize the space and should absolutely be called before really using this manager.
		virtual void partitionSpace() = 0;

		// Reorder the space using the passed particle Positions. This does not clear the former reordering but add the particle positions to the right partition.
		virtual void computeSpaceReordering(const std::vector<Storm::Vector3> &particlePositions, Storm::PartitionSelection modality, const unsigned int systemId) = 0;

		// Clear the space partition from all registered particle referrals for partition that aren't static. This does not remove the partition.
		virtual void clearSpaceReorderingNoStatic() = 0;

		// Clear the selected space partition from all registered particle referrals. This does not remove the partition.
		virtual void clearSpaceReorderingForPartition(Storm::PartitionSelection modality) = 0;

		// Get the all bundles that can be considered as neighbor from the bundle referred by systemId containing the particlePosition. 
		// Note that inOutContainingBundlePtr can also contain the particle at particlePosition.
		virtual void getAllBundles(const std::vector<Storm::NeighborParticleReferral>* &outContainingBundlePtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborBundle)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality) const = 0;

		// Get the containing bundle containing particlePosition.
		virtual void getContainingBundle(const std::vector<Storm::NeighborParticleReferral>* &containingBundlePtr, const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality) const = 0;

		// Set the partition length used when partitioning the space. The length is the length of one partition.
		// Beware since setting it will automatically reset the partitioning (recreate all partitions and clear the particle referrals).
		virtual void setPartitionLength(float length) = 0;
		virtual float getPartitionLength() const = 0;
	};
}
