#pragma once

#include "SingletonHeldInterfaceBase.h"

namespace Storm
{
	class ISpaceManager : public Storm::ISingletonHeldInterface<Storm::ISpaceManager>
	{
	public:
		virtual ~ISpaceManager() = default;

	public:
		// Partition the space into smaller chunks. This initialize the space and should absolutely be called before really using this manager.
		virtual void partitionSpace(unsigned int systemId, bool isFluid) = 0;

		// Reorder the space using the passed particle Positions. This clear the former reordering.
		virtual void computeSpaceReordering(const std::vector<Storm::Vector3> &particlePositions, unsigned int systemId, bool isFluid) = 0;

		// Get the all bundles that can be considered as neighbor from the bundle referred by systemId containing the particlePosition. 
		// Note that inOutContainingBundlePtr can also contain the particle at particlePosition.
		virtual void getAllBundles(const std::vector<std::size_t>* &outContainingBundlePtr, std::vector<const std::vector<std::size_t>*> &outNeighborBundle, const Storm::Vector3 &particlePosition, unsigned int systemId, bool isFluid) const = 0;

		virtual float getPartitionLength() const = 0;
		virtual void setPartitionLength(float length) = 0;
	};
}
