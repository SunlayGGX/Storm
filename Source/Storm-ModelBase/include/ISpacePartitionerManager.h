#pragma once

#include "SingletonHeldInterfaceBase.h"

#include "SpacePartitionConstants.h"

namespace Storm
{
	enum class PartitionSelection;
	struct NeighborParticleReferral;
	class IDistanceSpacePartitionProxy;
	class OutReflectedModality;

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

		// Get the all bundles that can be considered as neighbor from the bundle referred by systemId containing the particlePosition. 
		// Note that inOutContainingBundlePtr can also contain the particle at particlePosition.
		// This method is the overload that takes the infinite domain into account and should be called if isInfiniteDomainMode returns true instead of getAllBundles.
		// reflectModality is the reflected modality of the last call to getAllBundlesInfinite (in the current thread). We provide our own OutReflectedModality and reflectModality would point to it. But another run of getAllBundlesInfinite (or getAllBundles) and the variable will be modified.
		virtual void getAllBundlesInfinite(const std::vector<Storm::NeighborParticleReferral>* &outContainingBundlePtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborBundle)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality, const Storm::OutReflectedModality* &reflectModality) const = 0;

		// Get the containing bundle containing particlePosition.
		virtual void getContainingBundle(const std::vector<Storm::NeighborParticleReferral>* &containingBundlePtr, const Storm::Vector3 &particlePosition, Storm::PartitionSelection modality) const = 0;

		// Set the partition length used when partitioning the space. The length is the length of one partition.
		// Beware since setting it will automatically reset the partitioning (recreate all partitions and clear the particle referrals).
		virtual void setPartitionLength(float length) = 0;
		virtual float getPartitionLength() const = 0;

		// Create an additional temporary partition to hold posData.
		// Note that we don't keep a reference to the item, therefore you're responsible to clean it.
		virtual std::shared_ptr<Storm::IDistanceSpacePartitionProxy> makeDistancePartitionProxy(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, const float partitionLength) = 0;

		// Computes if the particle at position is outside the space domain. If true, then extra care should be made
		// (but in lot of case, do not continue to use the manager for such particle because we won't check inside any other methods
		// (we expect the check to be done separately before entering any of the other methods, or that you know what you are doing))
		virtual bool isOutsideSpaceDomain(const Storm::Vector3 &position) const = 0;

		// Returns a flag telling this manager is in infinite domain mode.
		// This method should be called before getting the bundle (to know if client should call getAllBundles or getAllBundlesInfinite).
		// Note that this flag is not checked. Therefore if the wrong method is called, we would just have the neighborhoods as if the feature was enabled/disabled.
		virtual bool isInfiniteDomainMode() const noexcept = 0;

		virtual Storm::Vector3 getDomainDimension() const noexcept = 0;
	};
}
