#pragma once

namespace
{
	static unsigned int computeBlocCountOnAxis(Storm::Vector3::Scalar diffCoordOnAxis, float voxelEdgeLength)
	{
		return static_cast<unsigned int>(diffCoordOnAxis / voxelEdgeLength) + 1;
	}

	template<class SelectorFunc>
	__forceinline unsigned int computeCoordIndexOnAxis(const Storm::Vector3ui &maxValue, const float voxelEdgeLength, const Storm::Vector3 &voxelShift, const Storm::Vector3 &position, const SelectorFunc &selectorFunc)
	{
		// if it over maxValue, then the particle has left the domain... Just register it inside the last voxel...
		// Whatever, the simulation was instable and we will lose a little performance but whatever, it's better than crashing...
		return std::min(selectorFunc(maxValue) - 1, static_cast<unsigned int>((selectorFunc(position) - selectorFunc(voxelShift)) / voxelEdgeLength));
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

	static PositionInDomain computePositionInDomain(const unsigned int xLast, const unsigned int yLast, const unsigned int zLast, const unsigned int xIndex, const unsigned int yIndex, const unsigned int zIndex)
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
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
				else // Is on back side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::RightTopBack;
				}
			}
		}
	}

	template<class VoxelType, class VoxelData>
	static void getVoxelsDataAtPositionImpl(const VoxelType &voxel, float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<VoxelData>* &outContainingVoxelPtr, const std::vector<VoxelData>*(&outNeighborData)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition)
	{
		unsigned int xIndex;
		unsigned int yIndex;
		unsigned int zIndex;

		const auto &gridBoundary = voxel.getGridBoundary();

		const auto &voxels = voxel.getVoxels();

		const std::size_t voxelIndex = static_cast<std::size_t>(voxel.computeRawIndexFromPosition(gridBoundary, voxelEdgeLength, voxelShift, particlePosition, xIndex, yIndex, zIndex));
		outContainingVoxelPtr = &voxels[voxelIndex].getData();

		const std::vector<VoxelData>** iter = std::begin(outNeighborData);

		const unsigned int xIndexBefore = xIndex - 1;
		const unsigned int yIndexBefore = yIndex - 1;
		const unsigned int zIndexBefore = zIndex - 1;
		const unsigned int xIndexAfter = xIndex + 1;
		const unsigned int yIndexAfter = yIndex + 1;
		const unsigned int zIndexAfter = zIndex + 1;

#define STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex) \
*iter = &voxels[voxel.computeRawIndexFromCoordIndex(xIndex, yIndex, zIndex)].getData(); \
++iter

		switch (computePositionInDomain(gridBoundary.x() - 1, gridBoundary.y() - 1, gridBoundary.z() - 1, xIndex, yIndex, zIndex))
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
		assert(std::end(outNeighborData) - iter != Storm::k_neighborLinkedBunkCount && "We have forgotten to handle a PositionInDomain enum value case!");

		// The next iterator should always be a nullptr to stop the loop. Since we have 1 more pointer than the maximum logically possible, we shouldn't trigger any sigsegv...
		*iter = nullptr;
	}
}
