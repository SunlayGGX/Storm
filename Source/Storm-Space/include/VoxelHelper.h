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

		MiddleMiddleFront,
		MiddleMiddleBack,
		LeftMiddleMiddle,
		RightMiddleMiddle,
		MiddleBottomMiddle,
		MiddleTopMiddle,

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
					return PositionInDomain::MiddleMiddleFront;
				}
				else // Is on back side, so the resulting enum value will contain the word "Back"
				{
					return PositionInDomain::MiddleMiddleBack;
				}
			}
			else if (yIndex == 0) // Is on bottom side, so the resulting enum value will contain the word "Bottom"
			{
				if (zIndex > 0 && zIndex < zLast) // z is Middle, so the resulting enum value will contain the word "Middle"
				{
					return PositionInDomain::MiddleBottomMiddle;
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
					return PositionInDomain::MiddleTopMiddle;
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
					return PositionInDomain::LeftMiddleMiddle;
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
					return PositionInDomain::RightMiddleMiddle;
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

	template<bool infiniteDomain = false, class VoxelType, class VoxelData>
	static void retrieveVoxelsDataAtPositionImpl(const VoxelType &voxel, float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<VoxelData>* &outContainingVoxelPtr, const std::vector<VoxelData>*(&outNeighborData)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition, bool &outShouldReflect)
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

		// In case of infinite domain. Neighborhoods are always filled to the brim.

#define STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex) \
*iter = &voxels[voxel.computeRawIndexFromCoordIndex(xIndex, yIndex, zIndex)].getData(); \
++iter

		switch (computePositionInDomain(gridBoundary.x() - 1, gridBoundary.y() - 1, gridBoundary.z() - 1, xIndex, yIndex, zIndex))
		{
		case PositionInDomain::AllMiddle: // We're inside the domain and not near a boundary, so complete neighborhood!
			outShouldReflect = false;
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

		case PositionInDomain::MiddleMiddleFront:
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

			if constexpr (infiniteDomain)
			{
				// Only reflect on Z. Since Z is front here, then we would need to include voxel at back Z.
				outShouldReflect = true;

				// Reflection on opposite Z Edge that is the Z before reflected from the first Z => on the last Z (The back).
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore_reflected);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore_reflected);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore_reflected);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::MiddleMiddleBack:
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

			if constexpr (infiniteDomain)
			{
				// Only reflect on Z. Since Z is back here, then we would need to include voxel at front Z.
				outShouldReflect = true;

				// Reflection on opposite Z Edge that is the Z before reflected from the last Z => on the first Z (The front) which is the Z with index 0.
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, 0);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, 0);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, 0);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::LeftMiddleMiddle:
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

			if constexpr (infiniteDomain)
			{
				// Only reflect on X. Since X is at the leftmost side of the domain here, then we would need to include voxel at the domain's rightmost side.
				outShouldReflect = true;

				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::RightMiddleMiddle:
			if constexpr (infiniteDomain)
			{
				// Only reflect on X. Since X is at the rightmost side of the domain here, then we would need to include voxel at the domain's leftmost side (aka x=0).
				outShouldReflect = true;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}

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

		case PositionInDomain::MiddleBottomMiddle:
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Only reflect on Y. Since Y is at the bottommost side of the domain here, then we would need to include voxel at the domain's topmost side.
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndexAfter);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexAfter);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::MiddleTopMiddle:
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Only reflect on Y. Since Y is at the topmost side of the domain here, then we would need to include voxel at the domain's bottommost side (aka y=0).
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndexAfter);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexAfter);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::LeftBottomFront:
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexAfter);

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;
				STORM_TODO("infinite domain -> Corner : Left bottom front");
				STORM_NOT_IMPLEMENTED;
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the X and Z. Leave Y Unchanged.
				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::LeftTopFront:
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexAfter);

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;
				STORM_TODO("infinite domain -> Corner : Left Top front");
				STORM_NOT_IMPLEMENTED;
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the X and Y. Leave Z Unchanged.
				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, 0, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::LeftTopBack:
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;
				STORM_TODO("infinite domain -> Corner : Left Top Back");
				STORM_NOT_IMPLEMENTED;
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the X and Z. Leave Y Unchanged.
				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, 0);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, 0);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndex);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::LeftBottomBack:
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndex);

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;
				STORM_TODO("infinite domain -> Corner : Left Bottom Back");
				STORM_NOT_IMPLEMENTED;
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the X and Y. Leave Z Unchanged.
				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndexAfter);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndex, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexAfter, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore_reflected, yIndexBefore_reflected, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::RightBottomFront:
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexAfter);

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;
				STORM_TODO("infinite domain -> Corner : Right Bottom Front");
				STORM_NOT_IMPLEMENTED;
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the X and Z. Leave Y Unchanged.
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore_reflected);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::RightTopFront:
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexAfter);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexAfter);

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;
				STORM_TODO("infinite domain -> Corner : Right Top Front");
				STORM_NOT_IMPLEMENTED;
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the X and Y. Leave Z Unchanged.
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, 0, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::RightTopBack:
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;
				STORM_TODO("infinite domain -> Corner : Right Top Back");
				STORM_NOT_IMPLEMENTED;
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the X and Z. Leave Y Unchanged.
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, 0);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		case PositionInDomain::RightBottomBack:
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndex);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore);
			STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndex);

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;
				STORM_TODO("infinite domain -> Corner : Right Bottom Back");
				STORM_NOT_IMPLEMENTED;
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the X and Y. Leave Z Unchanged.
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndex, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexAfter, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(0, yIndexBefore_reflected, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexAfter);
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the Y and Z. Leave X Unchanged.
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore_reflected);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore_reflected);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore_reflected);
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the Y and Z. Leave X Unchanged.
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndexBefore_reflected);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexBefore_reflected);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, zIndexBefore_reflected);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndexAfter);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndexBefore_reflected);
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the Y and Z. Leave X Unchanged.
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, 0);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, 0);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, 0, zIndex);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, 0);
			}
			else
			{
				outShouldReflect = false;
			}
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

			if constexpr (infiniteDomain)
			{
				outShouldReflect = true;

				// Edge : Reflect the Y and Z. Leave X Unchanged.
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexAfter, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexBefore, yIndexBefore_reflected, zIndex);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexAfter, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndexBefore_reflected, zIndex);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndex, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexAfter, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, 0);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndexBefore);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndexAfter, yIndexBefore_reflected, zIndex);
			}
			else
			{
				outShouldReflect = false;
			}
			break;

		default:
			Storm::throwException<Storm::Exception>("unknown position in domain value!");
			break;
		}

#undef STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR

		// The remaining unset are because they are outside the partitioned domain (happens when we consider a voxel at the domain boundary).
		// We must set those to nullptr since those neighborhoods don't exist.
		assert(std::end(outNeighborData) - iter != Storm::k_neighborLinkedBunkCount && "We have forgotten to handle a PositionInDomain enum value case!");

		if constexpr (infiniteDomain)
		{
			// On infinite domain, no matter the location of the voxel, we would always have a full neighborhood of voxels (the missing voxel neighbors would be provided from the opposite side reflection)
			enum { k_fullVoxelNeighborhood = Storm::k_neighborLinkedBunkCount - 1 };
			assert(std::end(outNeighborData) - iter == k_fullVoxelNeighborhood && "Infinite domain should reflect the voxel neighborhood in a way this one is always filled!");
		}

		// The next iterator should always be a nullptr to stop the loop. Since we have 1 more pointer than the maximum logically possible, we shouldn't trigger any sigsegv...
		*iter = nullptr;
	}
}
