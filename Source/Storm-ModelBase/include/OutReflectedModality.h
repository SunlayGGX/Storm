#pragma once

#include "BitField.h"
#include "SpacePartitionConstants.h"


namespace Storm
{
	namespace
	{
		template<auto val>
		constexpr static __forceinline uint8_t doOutReflectedModalityFlagChecks()
		{
			STORM_STATIC_ASSERT(val <= std::numeric_limits<uint8_t>::max(), "The flag value must be contained into a 8 bits integer. It is currently overflowing!");
			STORM_STATIC_ASSERT(val != static_cast<uint8_t>(0), "0 is a reserved value for OutReflectedModality flag (means no flag)!");
			return val;
		}
	}

#define STORM_DECLARE_MODALITY_FLAG(FlagName, ...) FlagName = doOutReflectedModalityFlagChecks<static_cast<uint8_t>(Storm::BitField<__VA_ARGS__>::value)>()

	enum class OutReflectedModalityEnum : uint8_t
	{
		None = 0x0,

		// We're on the right of the simulation domain and we did include neighborhood voxel reflected at the leftmost side of the domain
		STORM_DECLARE_MODALITY_FLAG(XReflectedToTheLeft,	1, 0, 0, 0, 0, 0),

		// We're on the top of the simulation domain and we did include neighborhood voxel reflected at the bottommost side of the domain
		STORM_DECLARE_MODALITY_FLAG(YReflectedToTheBottom,	0, 1, 0, 0, 0, 0),

		// We're on the back of the simulation domain and we did include neighborhood voxel reflected at the front side of the domain
		STORM_DECLARE_MODALITY_FLAG(ZReflectedToTheFront,	0, 0, 1, 0, 0, 0),

		// We're on the left of the simulation domain and we did include neighborhood voxel reflected at the rightmost side of the domain
		STORM_DECLARE_MODALITY_FLAG(XReflectedToTheRight,	0, 0, 0, 1, 0, 0),

		// We're on the bottom of the simulation domain and we did include neighborhood voxel reflected at the topmost side of the domain
		STORM_DECLARE_MODALITY_FLAG(YReflectedToTheTop,		0, 0, 0, 0, 1, 0),

		// We're on the front of the simulation domain and we did include neighborhood voxel reflected at the back side of the domain
		STORM_DECLARE_MODALITY_FLAG(ZReflectedToTheBack,	0, 0, 0, 0, 0, 1),
	};

#undef STORM_DECLARE_MODALITY_FLAG


	class OutReflectedModality
	{
	public:
		// Query _summary before using _modalityPerBundle. If it is None, then don't use _modalityPerBundle (we won't update it and rather update the summary in case it should be None for optimization purposes. Therefore using _modalityPerBundle while _summary is None could lead to using the outdated data.)
		Storm::OutReflectedModalityEnum _summary = Storm::OutReflectedModalityEnum::None;
		Storm::OutReflectedModalityEnum _modalityPerBundle[Storm::k_neighborLinkedBunkCount] = { Storm::OutReflectedModalityEnum::None };
	};
}
