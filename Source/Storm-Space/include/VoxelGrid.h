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
		VoxelGrid(const Storm::VoxelGrid &other);
		~VoxelGrid();

	public:
		void getVoxelsDataAtPosition(float voxelEdgeLength, const std::vector<Storm::NeighborParticleReferral>* &outContainingVoxelPtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborData)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition) const;

		void fill(float voxelEdgeLength, const std::vector<Storm::Vector3> &particlePositions, const unsigned int systemId);

		// Beware, this clear all data inside all voxels but not the voxels themselves (the space would remains partitioned, but without any particle inside).
		// To reset the partitioning, you must create a new VoxelGrid.
		void clear();

		std::size_t size() const;

	private:
		void computeCoordIndexFromPosition(float voxelEdgeLength, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const;
		unsigned int computeRawIndexFromCoordIndex(unsigned int xIndex, unsigned int yIndex, unsigned int zIndex) const;
		unsigned int computeRawIndexFromPosition(float voxelEdgeLength, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const;

	private:
		unsigned int _xVoxelCount;
		unsigned int _yVoxelCount;
		unsigned int _zVoxelCount;

		unsigned int _xIndexOffsetCoeff;

		// The raw buffer of voxel elements. It is ordered by x, then by y and finally by z.
		std::vector<Storm::Voxel> _voxels;
	};
}
