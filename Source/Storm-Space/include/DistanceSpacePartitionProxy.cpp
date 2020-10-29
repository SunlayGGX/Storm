#include "DistanceSpacePartitionProxy.h"

#include "PositionVoxel.h"

#include "VoxelHelper.h"
#include "Vector3Utils.h"

#include "ThrowException.h"



Storm::DistanceSpacePartitionProxy::DistanceSpacePartitionProxy(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, float voxelEdgeLength) :
	_voxelEdgeLength{ voxelEdgeLength }
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

	Storm::minNegativeInPlaceFromBoth(_gridShiftOffset, downCorner, upCorner, [](auto &vect) -> auto& { return vect.x(); });
	Storm::minNegativeInPlaceFromBoth(_gridShiftOffset, downCorner, upCorner, [](auto &vect) -> auto& { return vect.y(); });
	Storm::minNegativeInPlaceFromBoth(_gridShiftOffset, downCorner, upCorner, [](auto &vect) -> auto& { return vect.z(); });
}

Storm::DistanceSpacePartitionProxy::DistanceSpacePartitionProxy(const Storm::DistanceSpacePartitionProxy &other) :
	_gridBoundary{ other._gridBoundary },
	_xIndexOffsetCoeff{ other._xIndexOffsetCoeff },
	_voxels{ other._voxels },
	_voxelEdgeLength{ other._voxelEdgeLength }
{

}

Storm::DistanceSpacePartitionProxy::~DistanceSpacePartitionProxy() = default;

void Storm::DistanceSpacePartitionProxy::getBundleAtPosition(const std::vector<Storm::Vector3>* &outContainingBundlePtr, const std::vector<Storm::Vector3>*(&outNeighborBundle)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition) const
{
	getVoxelsDataAtPositionImpl(*this, _voxelEdgeLength, _gridShiftOffset, outContainingBundlePtr, outNeighborBundle, particlePosition);
}

void Storm::DistanceSpacePartitionProxy::addDataIfDistanceUnique(const Storm::Vector3 &data, const float distanceSquared)
{
	const std::vector<Storm::Vector3>* containingBundlePtr;
	const std::vector<Storm::Vector3>* neighborBundlePtr[Storm::k_neighborLinkedBunkCount];
	this->addDataIfDistanceUnique(data, distanceSquared, containingBundlePtr, neighborBundlePtr);
}

void Storm::DistanceSpacePartitionProxy::addDataIfDistanceUnique(const Storm::Vector3 &data, const float distanceSquared, const std::vector<Storm::Vector3>* &containingBundlePtr, const std::vector<Storm::Vector3>* (&neighborBundlePtr)[Storm::k_neighborLinkedBunkCount])
{
	this->getBundleAtPosition(containingBundlePtr, neighborBundlePtr, data);

	const auto searchLambda = [&data, distanceSquared](const std::vector<Storm::Vector3> &posContainers)
	{
		for (const Storm::Vector3 &pos : posContainers)
		{
#if true
			float acc = data.x() - pos.x();
			acc *= acc;
			if (acc < distanceSquared)
			{
				float tmp = data.y() - pos.y();
				acc += tmp * tmp;
				if (acc < distanceSquared)
				{
					tmp = data.z() - pos.z();
					if ((acc + tmp * tmp) < distanceSquared)
					{
						return true;
					}
				}
			}
#else
			if ((data - pos).squaredNorm() < distanceSquared)
			{
				return true;
			}
#endif
		}

		return false;
	};

	if (searchLambda(*containingBundlePtr))
	{
		return;
	}

	for (const std::vector<Storm::Vector3>*const* iter = neighborBundlePtr; *iter != nullptr; ++iter)
	{
		if (searchLambda(**iter))
		{
			return;
		}
	}

	unsigned int dummy1;
	unsigned int dummy2;
	unsigned int dummy3;

	const std::size_t voxelIndex = static_cast<std::size_t>(this->computeRawIndexFromPosition(_gridBoundary, _voxelEdgeLength, _gridShiftOffset, data, dummy1, dummy2, dummy3));
	Storm::PositionVoxel &voxel = _voxels[voxelIndex];
	voxel.addData(data);
}

std::vector<Storm::Vector3> Storm::DistanceSpacePartitionProxy::getCompleteData() const
{
	std::vector<Storm::Vector3> result;

	std::size_t expectedSize = 0;
	for (const Storm::PositionVoxel &voxel : _voxels)
	{
		expectedSize += voxel.getData().size();
	}
	result.reserve(expectedSize);

	for (const Storm::PositionVoxel &voxel : _voxels)
	{
		for (const Storm::Vector3 &pos : voxel.getData())
		{
			result.emplace_back(pos);
		}
	}

	return result;
}

std::size_t Storm::DistanceSpacePartitionProxy::size() const
{
	return _gridBoundary.x() * _gridBoundary.y() * _gridBoundary.z();
}

void Storm::DistanceSpacePartitionProxy::computeCoordIndexFromPosition(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const
{
	outXIndex = computeCoordIndexOnAxis(maxValue, voxelEdgeLength, voxelShift, position, [](auto &vect) -> auto& { return vect.x(); });
	outYIndex = computeCoordIndexOnAxis(maxValue, voxelEdgeLength, voxelShift, position, [](auto &vect) -> auto& { return vect.y(); });
	outZIndex = computeCoordIndexOnAxis(maxValue, voxelEdgeLength, voxelShift, position, [](auto &vect) -> auto& { return vect.z(); });
}

unsigned int Storm::DistanceSpacePartitionProxy::computeRawIndexFromCoordIndex(unsigned int xIndex, unsigned int yIndex, unsigned int zIndex) const
{
	return
		xIndex * _xIndexOffsetCoeff +
		yIndex * _gridBoundary.z() +
		zIndex
		;
}

unsigned int Storm::DistanceSpacePartitionProxy::computeRawIndexFromPosition(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const
{
	this->computeCoordIndexFromPosition(maxValue, voxelEdgeLength, voxelShift, position, outXIndex, outYIndex, outZIndex);

	const unsigned int result = this->computeRawIndexFromCoordIndex(outXIndex, outYIndex, outZIndex);
	assert(result < this->size() && "We referenced an index that goes outside the partitioned space. It is illegal!");

	return result;
}