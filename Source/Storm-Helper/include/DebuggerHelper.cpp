#include "DebuggerHelper.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX


void Storm::waitForDebuggerToAttach(bool breakAfter /*= false*/)
{
	while (::IsDebuggerPresent() == FALSE)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
	}

	if (breakAfter)
	{
		__debugbreak();
	}
}
