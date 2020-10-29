#pragma once

#include "IDistanceSpacePartitionProxy.h"


namespace Storm
{
	class PositionVoxel;

	class DistanceSpacePartitionProxy : public Storm::IDistanceSpacePartitionProxy
	{
	public:
		DistanceSpacePartitionProxy(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, float voxelEdgeLength);
		DistanceSpacePartitionProxy(Storm::DistanceSpacePartitionProxy &&other);
		DistanceSpacePartitionProxy(const Storm::DistanceSpacePartitionProxy &other);
		~DistanceSpacePartitionProxy();

	public:
		void getBundleAtPosition(const std::vector<Storm::Vector3>* &outContainingBundlePtr, const std::vector<Storm::Vector3>*(&outNeighborBundle)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition) const;

		void addDataIfDistanceUnique(const Storm::Vector3 &data, const float distanceSquared) final override;
		void addDataIfDistanceUnique(const Storm::Vector3 &data, const float distanceSquared, const std::vector<Storm::Vector3>* &containingBundlePtr, const std::vector<Storm::Vector3>* (&neighborBundlePtr)[Storm::k_neighborLinkedBunkCount]) final override;
		std::vector<Storm::Vector3> getCompleteData() const final override;

	public:
		__forceinline const std::vector<Storm::PositionVoxel>& getVoxels() const noexcept { return _voxels; }
		__forceinline const Storm::Vector3ui& getGridBoundary() const noexcept { return _gridBoundary; }

	public:
		std::size_t size() const;

		void computeCoordIndexFromPosition(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const;
		unsigned int computeRawIndexFromCoordIndex(unsigned int xIndex, unsigned int yIndex, unsigned int zIndex) const;
		unsigned int computeRawIndexFromPosition(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const;

	private:
		Storm::Vector3ui _gridBoundary;

		unsigned int _xIndexOffsetCoeff;
		float _voxelEdgeLength;

		Storm::Vector3 _gridShiftOffset;

		// The raw buffer of voxel elements. It is ordered by x, then by y and finally by z.
		std::vector<Storm::PositionVoxel> _voxels;

	};
}
