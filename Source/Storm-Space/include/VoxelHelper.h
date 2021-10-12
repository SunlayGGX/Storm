#pragma once

#include "OutReflectedModality.h"

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

	using OutReflectedModalityEnumUnderlyingNative = Storm::EnumUnderlyingNative<Storm::OutReflectedModalityEnum>;

	STORM_STATIC_ASSERT(static_cast<OutReflectedModalityEnumUnderlyingNative>(Storm::OutReflectedModalityEnum::None) == 0, "Storm::OutReflectedModalityEnum::None should have the value 0x0 to be composed adequately.");

#define STORM_COMPOSE_REFLECTED_BITS(XFlag, YFlag, ZFlag) static_cast<OutReflectedModalityEnumUnderlyingNative>(Storm::OutReflectedModalityEnum::XFlag) | static_cast<OutReflectedModalityEnumUnderlyingNative>(Storm::OutReflectedModalityEnum::YFlag) | static_cast<OutReflectedModalityEnumUnderlyingNative>(Storm::OutReflectedModalityEnum::ZFlag)

	enum class MakeSummaryReflectedComposeFrom : OutReflectedModalityEnumUnderlyingNative
	{
		// xyz

		MiddleMiddleFront = STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack),
		MiddleMiddleBack = STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack),
		LeftMiddleMiddle = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None),
		RightMiddleMiddle = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None),
		MiddleBottomMiddle = STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None),
		MiddleTopMiddle = STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None),

		LeftBottomFront = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, ZReflectedToTheBack),
		LeftMiddleFront = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheBack),
		LeftTopFront = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, ZReflectedToTheBack),
		LeftTopMiddle = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, None),
		LeftTopBack = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, ZReflectedToTheFront),
		LeftMiddleBack = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheFront),
		LeftBottomBack = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, ZReflectedToTheFront),
		LeftBottomMiddle = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, None),

		RightBottomFront = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, ZReflectedToTheBack),
		RightMiddleFront = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheBack),
		RightTopFront = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, ZReflectedToTheBack),
		RightTopMiddle = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, None),
		RightTopBack = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, ZReflectedToTheFront),
		RightMiddleBack = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheFront),
		RightBottomBack = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, ZReflectedToTheFront),
		RightBottomMiddle = STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, None),

		MiddleTopFront = STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheBack),
		MiddleBottomFront = STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheBack),
		MiddleTopBack = STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheFront),
		MiddleBottomBack = STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheFront),
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
	static void retrieveVoxelsDataAtPositionImpl(const VoxelType &voxel, float voxelEdgeLength, const Storm::Vector3 &voxelShift, const std::vector<VoxelData>* &outContainingVoxelPtr, const std::vector<VoxelData>*(&outNeighborData)[Storm::k_neighborLinkedBunkCount], const Storm::Vector3 &particlePosition, const Storm::OutReflectedModality* &reflectModality)
	{
		thread_local Storm::OutReflectedModality reflectedModalityPerThread;
		reflectModality = &reflectedModalityPerThread;

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

#define STORM_CUT_REFLECTED_MODALITY_BUNDLE(caseFlag)																					\
Storm::OutReflectedModalityEnum* reflectedModalityIter;																					\
{																																		\
	const std::size_t cutPosition = iter - outNeighborData;																				\
	::memset(																															\
		reflectedModalityPerThread._modalityPerBundle,																					\
		static_cast<int>(Storm::OutReflectedModalityEnum::None),																		\
		cutPosition * sizeof(*std::begin(reflectedModalityPerThread._modalityPerBundle))												\
	);																																	\
	reflectedModalityIter = reflectedModalityPerThread._modalityPerBundle + cutPosition;												\
}																																		\
reflectedModalityPerThread._summary = static_cast<Storm::OutReflectedModalityEnum>(MakeSummaryReflectedComposeFrom::caseFlag)

#define STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex) \
*iter = &voxels[voxel.computeRawIndexFromCoordIndex(xIndex, yIndex, zIndex)].getData(); \
++iter

#define STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndex, reflectionModalityFlag) \
STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR(xIndex, yIndex, zIndex); \
*reflectedModalityIter = static_cast<Storm::OutReflectedModalityEnum>(reflectionModalityFlag); \
++reflectedModalityIter

		switch (computePositionInDomain(gridBoundary.x() - 1, gridBoundary.y() - 1, gridBoundary.z() - 1, xIndex, yIndex, zIndex))
		{
		case PositionInDomain::AllMiddle: // We're inside the domain and not near a boundary, so complete neighborhood!
			reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(MiddleMiddleFront);

				// Reflection on opposite Z Edge that is the Z before reflected from the first Z => on the last Z (The back).
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexAfter, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexAfter, zIndexBefore_reflected, MakeSummaryReflectedComposeFrom::MiddleMiddleFront);
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(MiddleMiddleBack);

				// Reflection on opposite Z Edge that is the Z before reflected from the last Z => on the first Z (The front) which is the Z with index 0.
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexAfter, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexAfter, 0, MakeSummaryReflectedComposeFrom::MiddleMiddleBack);
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftMiddleMiddle);

				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexBefore, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndex, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexAfter, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexAfter, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexBefore, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndex, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexAfter, MakeSummaryReflectedComposeFrom::LeftMiddleMiddle);
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
			}
			break;

		case PositionInDomain::RightMiddleMiddle:
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

			if constexpr (infiniteDomain)
			{
				// Only reflect on X. Since X is at the rightmost side of the domain here, then we would need to include voxel at the domain's leftmost side (aka x=0).
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightMiddleMiddle);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexBefore, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndex, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexAfter, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexAfter, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexBefore, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndex, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexAfter, MakeSummaryReflectedComposeFrom::RightMiddleMiddle);
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
			}

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
				// Only reflect on Y. Since Y is at the bottommost side of the domain here, then we would need to include voxel at the domain's topmost side.
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(MiddleBottomMiddle);

				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexBefore, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndex, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexAfter, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexAfter, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexBefore, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndex, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexAfter, MakeSummaryReflectedComposeFrom::MiddleBottomMiddle);
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				// Only reflect on Y. Since Y is at the topmost side of the domain here, then we would need to include voxel at the domain's bottommost side (aka y=0).
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(MiddleTopMiddle);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexBefore, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndex, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexAfter, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexAfter, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexBefore, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndex, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexAfter, MakeSummaryReflectedComposeFrom::MiddleTopMiddle);
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftBottomFront);

				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheBack));
				
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, ZReflectedToTheBack));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftMiddleFront);

				// Edge : Reflect the X and Z. Leave Y Unchanged.
				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftTopFront);

				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheBack));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftTopMiddle);

				// Edge : Reflect the X and Y. Leave Z Unchanged.
				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftTopBack);

				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftMiddleBack);

				// Edge : Reflect the X and Z. Leave Y Unchanged.
				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftBottomBack);

				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(LeftBottomMiddle);

				// Edge : Reflect the X and Y. Leave Z Unchanged.
				const unsigned int xIndexBefore_reflected = gridBoundary.x() - 1;
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexAfter, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore_reflected, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheRight, YReflectedToTheTop, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightBottomFront);

				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheBack));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightMiddleFront);

				// Edge : Reflect the X and Z. Leave Y Unchanged.
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightTopFront);

				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightTopMiddle);

				// Edge : Reflect the X and Y. Leave Z Unchanged.
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightTopBack);

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightMiddleBack);

				// Edge : Reflect the X and Z. Leave Y Unchanged.
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightBottomBack);

				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, None));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(RightBottomMiddle);

				// Edge : Reflect the X and Y. Leave Z Unchanged.
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndex, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexAfter, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, None, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(0, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(XReflectedToTheLeft, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(MiddleTopFront);

				// Edge : Reflect the Y and Z. Leave X Unchanged.
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(MiddleBottomFront);

				// Edge : Reflect the Y and Z. Leave X Unchanged.
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;
				const unsigned int zIndexBefore_reflected = gridBoundary.z() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheBack));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexAfter, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheBack));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexAfter, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexBefore_reflected, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheBack));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(MiddleTopBack);

				// Edge : Reflect the Y and Z. Leave X Unchanged.
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, 0, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheBottom, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
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
				STORM_CUT_REFLECTED_MODALITY_BUNDLE(MiddleBottomBack);

				// Edge : Reflect the Y and Z. Leave X Unchanged.
				const unsigned int yIndexBefore_reflected = gridBoundary.y() - 1;

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexBefore, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndex, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));

				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndex, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexAfter, 0, STORM_COMPOSE_REFLECTED_BITS(None, None, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, 0, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, ZReflectedToTheFront));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndexBefore, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
				STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED(xIndexAfter, yIndexBefore_reflected, zIndex, STORM_COMPOSE_REFLECTED_BITS(None, YReflectedToTheTop, None));
			}
			else
			{
				reflectedModalityPerThread._summary = Storm::OutReflectedModalityEnum::None;
			}
			break;

		default:
			Storm::throwException<Storm::Exception>("unknown position in domain value!");
			break;
		}

#undef STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR_REFLECTED
#undef STORM_ATTRIBUTE_VALUES_TO_NEIGHBOR_DATA_ITERATOR
#undef STORM_CUT_REFLECTED_MODALITY_BUNDLE

		// The remaining unset are because they are outside the partitioned domain (happens when we consider a voxel at the domain boundary).
		// We must set those to nullptr since those neighborhoods don't exist.
		assert(std::end(outNeighborData) - iter != Storm::k_neighborLinkedBunkCount && "We have forgotten to handle a PositionInDomain enum value case!");

		if constexpr (infiniteDomain)
		{
			// On infinite domain, no matter the location of the voxel, we would always have a full neighborhood of voxels (the missing voxel neighbors would be provided from the opposite side reflection)
			enum 
			{
				k_trueEndBundleWithNullBundlePos = Storm::k_neighborLinkedBunkCount - 1
			};
			assert(iter - outNeighborData == k_trueEndBundleWithNullBundlePos && "Infinite domain should reflect the voxel neighborhood in a way this one is always filled!");
		}

		// The next iterator should always be a nullptr to stop the loop. Since we have 1 more pointer than the maximum logically possible, we shouldn't trigger any sigsegv...
		*iter = nullptr;
	}
}

#undef STORM_COMPOSE_REFLECTED_BITS
