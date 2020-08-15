#include "VoxelGrid.h"

#include "StormMacro.h"
#include "ThrowException.h"


Storm::VoxelGrid::VoxelGrid(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, float voxelEdgeLength)
{
	STORM_NOT_IMPLEMENTED;
}

void Storm::VoxelGrid::getVoxelsDataAtPosition(const std::vector<std::size_t>* &outContainingVoxelPtr, std::vector<const std::vector<std::size_t>*> &outNeighborData, const Storm::Vector3 &particlePosition) const
{
	STORM_NOT_IMPLEMENTED;
}

void Storm::VoxelGrid::fill(const std::vector<Storm::Vector3> &particlePositions)
{
	STORM_NOT_IMPLEMENTED;
}

void Storm::VoxelGrid::clear()
{
	STORM_NOT_IMPLEMENTED;
}
