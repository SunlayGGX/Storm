#pragma once

#include "SpacePartitionConstants.h"


namespace Storm
{
	class Voxel;
	struct NeighborParticleReferral;

	class VoxelGrid
	{
	public:
		VoxelGrid(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, float voxelEdgeLength);
		VoxelGrid(Storm::VoxelGrid &&other);
		VoxelGrid(const Storm::VoxelGrid &other);
		~VoxelGrid();

	public:
		void getVoxelsDataAtPosition(float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<Storm::NeighborParticleReferral>* &outContainingVoxelPtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborData)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition) const;
		void getVoxelsDataAtPosition(float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<Storm::NeighborParticleReferral>* &outContainingVoxelPtr, const Storm::Vector3 &particlePosition) const;

		void fill(float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<Storm::Vector3> &particlePositions, const unsigned int systemId);

		// Beware, this clear all data inside all voxels but not the voxels themselves (the space would remains partitioned, but without any particle inside).
		// To reset the partitioning, you must create a new VoxelGrid.
		void clear();

		std::size_t size() const;

		__forceinline const std::vector<Storm::Voxel>& getVoxels() const noexcept { return _voxels; }
		__forceinline const Storm::Vector3ui& getGridBoundary() const noexcept { return _gridBoundary; }

	public:
		void computeCoordIndexFromPosition(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const;
		unsigned int computeRawIndexFromCoordIndex(unsigned int xIndex, unsigned int yIndex, unsigned int zIndex) const;
		unsigned int computeRawIndexFromPosition(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const;

	public:
		std::vector<Storm::Voxel*> getVoxelsUnderRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, const float minDist, const float maxDist, const float voxelEdgeLength) const;

	private:
		Storm::Vector3ui _gridBoundary;

		unsigned int _xIndexOffsetCoeff;

		// The raw buffer of voxel elements. It is ordered by x, then by y and finally by z.
		std::vector<Storm::Voxel> _voxels;
	};
}
