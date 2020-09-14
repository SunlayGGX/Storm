#include "DebuggerHelper.h"

#include "LeanWindowsInclude.h"


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
