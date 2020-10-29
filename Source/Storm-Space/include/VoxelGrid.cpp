#include "VoxelGrid.h"

#include "StormMacro.h"
#include "ThrowException.h"

#include "Voxel.h"
#include "MemoryHelper.h"

#include "VoxelHelper.h"


Storm::VoxelGrid::VoxelGrid(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, float voxelEdgeLength)
{
	if (voxelEdgeLength < 0.00000001f || isnan(voxelEdgeLength) || isinf(voxelEdgeLength))
	{
		assert(false && "Invalid voxel length! This is forbidden!");
		Storm::throwException<std::exception>("Invalid voxel length (" + std::to_string(voxelEdgeLength) + ")! This is forbidden!");
	}

	const Storm::Vector3 diff = upCorner - downCorner;

	_gridBoundary.x() = computeBlocCountOnAxis(diff.x(), voxelEdgeLength);
	_gridBoundary.y() = computeBlocCountOnAxis(diff.y(), voxelEdgeLength);
	_gridBoundary.z() = computeBlocCountOnAxis(diff.z(), voxelEdgeLength);

	_xIndexOffsetCoeff = _gridBoundary.y() * _gridBoundary.z();

	_voxels.resize(this->size());
}

Storm::VoxelGrid::VoxelGrid(Storm::VoxelGrid &&other) = default;
Storm::VoxelGrid::VoxelGrid(const Storm::VoxelGrid &other) = default;
Storm::VoxelGrid::~VoxelGrid() = default;

void Storm::VoxelGrid::getVoxelsDataAtPosition(float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<Storm::NeighborParticleReferral>* &outContainingVoxelPtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborData)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition) const
{
	getVoxelsDataAtPositionImpl(*this, voxelEdgeLength, voxelShift, outContainingVoxelPtr, outNeighborData, particlePosition);
}

void Storm::VoxelGrid::getVoxelsDataAtPosition(float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<Storm::NeighborParticleReferral>* &outContainingVoxelPtr, const Storm::Vector3 &particlePosition) const
{
	unsigned int xIndex;
	unsigned int yIndex;
	unsigned int zIndex;

	const std::size_t voxelIndex = static_cast<std::size_t>(this->computeRawIndexFromPosition(_gridBoundary, voxelEdgeLength, voxelShift, particlePosition, xIndex, yIndex, zIndex));
	outContainingVoxelPtr = &_voxels[voxelIndex].getData();
}

void Storm::VoxelGrid::fill(float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<Storm::Vector3> &particlePositions, const unsigned int systemId)
{
	unsigned int dummy1;
	unsigned int dummy2;
	unsigned int dummy3;

	const std::size_t particleCount = particlePositions.size();
	for (std::size_t index = 0; index < particleCount; ++index)
	{
		const std::size_t voxelIndex = static_cast<std::size_t>(this->computeRawIndexFromPosition(_gridBoundary, voxelEdgeLength, voxelShift, particlePositions[index], dummy1, dummy2, dummy3));
		_voxels[voxelIndex].addParticle(index, systemId);
	}
}

void Storm::VoxelGrid::clear()
{
	for (Storm::Voxel &voxel : _voxels)
	{
		voxel.clear();
	}
}

std::size_t Storm::VoxelGrid::size() const
{
	return _gridBoundary.x() * _gridBoundary.y() * _gridBoundary.z();
}

void Storm::VoxelGrid::computeCoordIndexFromPosition(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const
{
	outXIndex = computeCoordIndexOnAxis(maxValue, voxelEdgeLength, voxelShift, position, [](auto &vect) -> auto& { return vect.x(); });
	outYIndex = computeCoordIndexOnAxis(maxValue, voxelEdgeLength, voxelShift, position, [](auto &vect) -> auto& { return vect.y(); });
	outZIndex = computeCoordIndexOnAxis(maxValue, voxelEdgeLength, voxelShift, position, [](auto &vect) -> auto& { return vect.z(); });
}

unsigned int Storm::VoxelGrid::computeRawIndexFromCoordIndex(unsigned int xIndex, unsigned int yIndex, unsigned int zIndex) const
{
	return 
		xIndex * _xIndexOffsetCoeff +
		yIndex * _gridBoundary.z() +
		zIndex
		;
}

unsigned int Storm::VoxelGrid::computeRawIndexFromPosition(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const
{
	this->computeCoordIndexFromPosition(maxValue, voxelEdgeLength, voxelShift, position, outXIndex, outYIndex, outZIndex);

	const unsigned int result = this->computeRawIndexFromCoordIndex(outXIndex, outYIndex, outZIndex);
	assert(result < this->size() && "We referenced an index that goes outside the partitioned space. It is illegal!");

	return result;
}

std::vector<Storm::Voxel*> Storm::VoxelGrid::getVoxelsUnderRaycast(const Storm::Vector3 &origin, const Storm::Vector3 &direction, const float minDist, const float maxDist, const float voxelEdgeLength) const
{
	STORM_NOT_IMPLEMENTED;
}
