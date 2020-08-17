#include "VoxelGrid.h"

#include "StormMacro.h"
#include "ThrowException.h"

#include "Voxel.h"
#include "MemoryHelper.h"

namespace
{
	unsigned int computeBlocCountOnAxis(Storm::Vector3::Scalar diffCoordOnAxis, float voxelEdgeLength)
	{
		return static_cast<unsigned int>(diffCoordOnAxis / voxelEdgeLength) + 1;
	}

	enum class PositionInDomain
	{
		// x=middle, y=middle, z=middle
		AllMiddle,

		MiddleFront,
		MiddleBack,
		MiddleLeft,
		MiddleRight,
		MiddleBottom,
		MiddleTop,

		// xyz

		// x=left, y=bottom/top, z=front/back
		LeftBottomFront,
		LeftMiddleFront,
		LeftTopFront,
		LeftTopMiddle,
		LeftTopBack,
		LeftMiddleBack,
		LeftBottomBack,
		LeftBottomMiddle,

		// x=right, y=bottom/top, z=front/back
		RightBottomFront,
		RightMiddleFront,
		RightTopFront,
		RightTopMiddle,
		RightTopBack,
		RightMiddleBack,
		RightBottomBack,
		RightBottomMiddle,

		// Remaining on x middle
		MiddleTopFront,
		MiddleBottomFront,
		MiddleTopBack,
		MiddleBottomBack,
	};

	PositionInDomain computePositionInDomain(const unsigned int xLast, const unsigned int yLast, const unsigned int zLast, const unsigned int xIndex, const unsigned int yIndex, const unsigned int zIndex)
	{
		if (xIndex > 0 && xIndex < xLast) // x is Middle, so the resulting enum value will contain the word "Middle"
		{
			if (yIndex > 0 && yIndex < yLast) // y is Middle, so the resulting enum value will contain the word "Middle"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::AllMiddle;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::MiddleFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::MiddleBack;
				}
			}
			else if (yIndex == 0) // Is on bottom side, so the resulting enum value will contain the word "Bottom"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::MiddleBottom;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::MiddleBottomFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::MiddleBottomBack;
				}
			}
			else // Is on top side, so the resulting enum value will contain the word "Top"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::MiddleTop;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::MiddleTopFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::MiddleTopBack;
				}
			}
		}
		else if (xIndex == 0) // Is on left side, so the resulting enum value will contain the word "Left"
		{
			if (yIndex > 0 && yIndex < yLast) // y is Middle, so the resulting enum value will contain the word "Middle"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::MiddleLeft;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::LeftMiddleFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::LeftMiddleBack;
				}
			}
			else if (yIndex == 0) // Is on bottom side, so the resulting enum value will contain the word "Bottom"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::LeftBottomMiddle;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::LeftBottomFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::LeftBottomBack;
				}
			}
			else // Is on top side, so the resulting enum value will contain the word "Top"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::LeftTopMiddle;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::LeftTopFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::LeftTopBack;
				}
			}
		}
		else // Is on right side, so the resulting enum value will contain the word "Right"
		{
			if (yIndex > 0 && yIndex < yLast) // y is Middle, so the resulting enum value will contain the word "Middle"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::MiddleRight;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::RightMiddleFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::RightMiddleBack;
				}
			}
			else if (yIndex == 0) // Is on bottom side, so the resulting enum value will contain the word "Bottom"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::RightBottomMiddle;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::RightBottomFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::RightBottomBack;
				}
			}
			else // Is on top side, so the resulting enum value will contain the word "Top"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::RightTopMiddle;
				}
				else if (zIndex == 0) // Is on front side, so the resulting enum value will contain the word "Front"
				{
					return PositionInDomain::RightTopFront;
				}
				else // Is on top side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::RightTopBack;
				}
			}
		}
	}
}


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

	for (unsigned int xIter = 0; xIter < _gridBoundary.x(); ++xIter)
	{
		for (unsigned int yIter = 0; yIter < _gridBoundary.y(); ++yIter)
		{
			for (unsigned int zIter = 0; zIter < _gridBoundary.z(); ++zIter)
			{
				_voxels.emplace_back();
			}
		}
	}
}

Storm::VoxelGrid::VoxelGrid(const Storm::VoxelGrid &other) :
	_gridBoundary{ other._gridBoundary },
	_xIndexOffsetCoeff{ other._xIndexOffsetCoeff },
	_voxels{ other._voxels }
{

}

Storm::VoxelGrid::~VoxelGrid() = default;

void Storm::VoxelGrid::getVoxelsDataAtPosition(float voxelEdgeLength, const std::vector<Storm::NeighborParticleReferral>* &outContainingVoxelPtr, const std::vector<Storm::NeighborParticleReferral>*(&outNeighborData)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition) const
{
	unsigned int xIndex;
	unsigned int yIndex;
	unsigned int zIndex;

	const std::size_t voxelIndex = static_cast<std::size_t>(this->computeRawIndexFromPosition(voxelEdgeLength, particlePosition, xIndex, yIndex, zIndex));
	outContainingVoxelPtr = &_voxels[voxelIndex].getData();

	const std::vector<Storm::NeighborParticleReferral>** iter = std::begin(outNeighborData);

	const unsigned int xIndexBefore = xIndex - 1;
	const unsigned int yIndexBefore = yIndex - 1;
	const unsigned int zIndexBefore = zIndex - 1;
	const unsigned int xIndexAfter = xIndex + 1;
	const unsigned int yIndexAfter = yIndex + 1;
	const unsigned int zIndexAfter = zIndex + 1;

#define STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex) \
*iter = &_voxels[this->computeRawIndexFromCoordIndex(xIndex, yIndex, zIndex)].getData(); \
++iter

	switch (computePositionInDomain(_gridBoundary.x() - 1, _gridBoundary.y() - 1, _gridBoundary.z() - 1, xIndex, yIndex, zIndex))
	{
	case PositionInDomain::AllMiddle: // We're inside the domain and not near a boundary, so complete neighborhood!
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		//STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::MiddleFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		//STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::MiddleBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		//STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		break;

	case PositionInDomain::MiddleLeft:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		//STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::MiddleRight:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		//STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::MiddleBottom:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		//STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::MiddleTop:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		//STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);

		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		break;

	case PositionInDomain::LeftBottomFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::LeftMiddleFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::LeftTopFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		break;

	case PositionInDomain::LeftTopMiddle:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		break;

	case PositionInDomain::LeftTopBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		break;

	case PositionInDomain::LeftMiddleBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		break;

	case PositionInDomain::LeftBottomBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		break;

	case PositionInDomain::LeftBottomMiddle:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::RightBottomFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::RightMiddleFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::RightTopFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		break;

	case PositionInDomain::RightTopMiddle:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		break;

	case PositionInDomain::RightTopBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		break;

	case PositionInDomain::RightMiddleBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		break;

	case PositionInDomain::RightBottomBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		break;

	case PositionInDomain::RightBottomMiddle:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::MiddleTopFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		break;

	case PositionInDomain::MiddleBottomFront:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);
		break;

	case PositionInDomain::MiddleTopBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		break;

	case PositionInDomain::MiddleBottomBack:
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
		STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
		break;

	default:
		Storm::throwException<std::exception>("unknown position in domain value!");
		break;
	}

#undef STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR

	// The remaining unset are because they are outside the partitioned domain (happens when we consider a voxel at the domain boundary).
	// We must set those to nullptr since those neighborhood don't exist.
	const std::size_t ghostNeighborhoodCount = std::end(outNeighborData) - iter;
	assert(ghostNeighborhoodCount != Storm::k_neighborLinkedBunkCount && "We have forgotten to handle a PositionInDomain enum value case!");

	memset(iter, NULL, ghostNeighborhoodCount);
}

void Storm::VoxelGrid::fill(float voxelEdgeLength, const std::vector<Storm::Vector3> &particlePositions, const unsigned int systemId)
{
	unsigned int dummy1;
	unsigned int dummy2;
	unsigned int dummy3;

	const std::size_t particleCount = particlePositions.size();
	for (std::size_t index = 0; index < particleCount; ++index)
	{
		const std::size_t voxelIndex = static_cast<std::size_t>(this->computeRawIndexFromPosition(voxelEdgeLength, particlePositions[index], dummy1, dummy2, dummy3));
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

void Storm::VoxelGrid::computeCoordIndexFromPosition(float voxelEdgeLength, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const
{
	outXIndex = static_cast<unsigned int>(position.x() / voxelEdgeLength);
	outYIndex = static_cast<unsigned int>(position.y() / voxelEdgeLength);
	outZIndex = static_cast<unsigned int>(position.z() / voxelEdgeLength);
}

unsigned int Storm::VoxelGrid::computeRawIndexFromCoordIndex(unsigned int xIndex, unsigned int yIndex, unsigned int zIndex) const
{
	return 
		xIndex * _xIndexOffsetCoeff +
		yIndex * _gridBoundary.z() +
		zIndex
		;
}

unsigned int Storm::VoxelGrid::computeRawIndexFromPosition(float voxelEdgeLength, const Storm::Vector3 &position, unsigned int &outXIndex, unsigned int &outYIndex, unsigned int &outZIndex) const
{
	this->computeCoordIndexFromPosition(voxelEdgeLength, position, outXIndex, outYIndex, outZIndex);

	const unsigned int result = this->computeRawIndexFromCoordIndex(outXIndex, outYIndex, outZIndex);
	assert(result < this->size() && "We referenced an index that goes outside the partitioned space. It is illegal!");

	return result;
}
