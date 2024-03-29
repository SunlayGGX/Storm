#pragma once

#include "BitField.h"


namespace Storm
{
	namespace
	{
		template<uint64_t val>
		constexpr static __forceinline uint64_t doThreadFlagChecks()
		{
			STORM_STATIC_ASSERT(val != static_cast<uint64_t>(0), "0 is a reserved value for threading flag (means no flag)!");
			return val;
		}
	}

#define STORM_DECLARE_THREAD_FLAG(FlagName, ...) FlagName = doThreadFlagChecks<static_cast<uint64_t>(Storm::BitField<__VA_ARGS__>::value)>()

	enum class ThreadFlagEnum : uint64_t
	{
		NoFlag = 0x0,

		STORM_DECLARE_THREAD_FLAG(MainThread,			1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(WindowsThread,		0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(InputThread,			0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(GraphicThread,		0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(LoadingThread,		0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(RaycastThread,		0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(SimulationThread,		0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(LoggingThread,		0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(SerializingThread,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(ScriptingThread,		0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(TimeThread,			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(SpaceThread,			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(PhysicsThread,		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(AnimationThread,		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0	),
		STORM_DECLARE_THREAD_FLAG(NetworkThread,		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0	),
		STORM_DECLARE_THREAD_FLAG(SafetyThread,			0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1	),
	};

#undef STORM_DECLARE_THREAD_FLAG
}
