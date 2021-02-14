#include "ThreadingSafety.h"

#include "ThreadingFlagger.h"

#include "ThreadFlagEnum.h"


namespace
{
	__forceinline static Storm::ThreadFlagEnum& refQueryCurrentThreadFlag() noexcept
	{
		thread_local Storm::ThreadFlagEnum currentThreadFlag = Storm::ThreadFlagEnum::NoFlag;
		return currentThreadFlag;
	}

	template<Storm::ThreadFlagEnum flag>
	__forceinline static bool currentThreadContainsFlag()
	{
		return (static_cast<uint64_t>(refQueryCurrentThreadFlag()) & static_cast<uint64_t>(flag)) != static_cast<uint64_t>(0);
	}
}


void Storm::ThreadingFlagger::addThreadFlag(const Storm::ThreadFlagEnum threadFlag) noexcept
{
	Storm::ThreadFlagEnum &currentThreadFlag = refQueryCurrentThreadFlag();
	currentThreadFlag = static_cast<Storm::ThreadFlagEnum>(static_cast<uint64_t>(currentThreadFlag) | static_cast<uint64_t>(threadFlag));
}


bool Storm::isMainThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::MainThread>();
}

bool Storm::isSimulationThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::SimulationThread>();
}

bool Storm::isLoadingThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::LoadingThread>();
}

bool Storm::isWindowsThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::WindowsThread>();
}

bool Storm::isTimeThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::TimeThread>();
}

bool Storm::isGraphicThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::GraphicThread>();
}

bool Storm::isRaycastThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::RaycastThread>();
}

bool Storm::isSpaceThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::SpaceThread>();
}

bool Storm::isSerializerThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::SerializingThread>();
}

bool Storm::isInputThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::InputThread>();
}

bool Storm::isLoggerThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::LoggingThread>();
}

bool Storm::isScriptThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::ScriptingThread>();
}

bool Storm::isPhysicsThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::PhysicsThread>();
}

bool Storm::isAnimationThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::AnimationThread>();
}

bool Storm::isNetworkThread()
{
	return currentThreadContainsFlag<Storm::ThreadFlagEnum::NetworkThread>();
}
